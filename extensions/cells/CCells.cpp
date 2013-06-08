/****************************************************************************
 Copyright (c) 2012-2013 Kevin Sun and RenRen Games

 email:happykevins@gmail.com
 http://wan.renren.com
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "CCells.h"

#include <assert.h>
#include <stdio.h>
#include "CCell.h"
#include "CUtils.h"
#include "CCreationFactory.h"

namespace cells
{

const double CELLS_WORKING_LOAD = 0.03f;

static pthread_t s_thread;
static volatile bool s_running = false;

void* CCells::cells_working(void* context)
{
	CCells* cells = (CCells*)context;
	assert(cells->regulation().auto_dispatch);

	double last_time = CUtils::gettime_seconds();
	double this_time = .0f;
	double delta_time = .0f;
	while( s_running )
	{
		if ( cells->is_suspend() )
		{
			continue;
		}

		this_time = CUtils::gettime_seconds();

		delta_time = this_time - last_time;
		if ( delta_time < CELLS_WORKING_LOAD )
		{
			CUtils::sleep( (unsigned int) ((CELLS_WORKING_LOAD - delta_time) * 1000) );
			delta_time = CELLS_WORKING_LOAD;
		}

		last_time = this_time;

		cells->tick_dispatch(delta_time);
	}

	return NULL;
}

CCells::CCells() :
		m_factory(NULL), m_suspend(true)
{

}

bool CCells::initialize(const CRegulation& rule)
{
	if ( m_factory )
		return false;

	m_rule = rule;

	// 有效性验证
	if ( m_rule.worker_thread_num < 1 ||  m_rule.worker_thread_num > 32 )
	{
		CLogE("[Cells Error]: invalid worker num %d!\n", (int)m_rule.worker_thread_num );
		return false;
	}

	if ( CUtils::str_trim(m_rule.tempfile_suffix).empty() )
	{
		CLogE("[Cells Error]: invalid suffix name %s!\n", m_rule.tempfile_suffix.c_str() );
		return false;
	}

	m_factory = new CCreationFactory(this, m_rule.worker_thread_num);

	s_running = true;
	if ( m_rule.auto_dispatch )
		pthread_create(&s_thread, NULL, CCells::cells_working, this);

	resume();

	return true;
}

void CCells::destroy()
{
	s_running = false;
	if ( m_rule.auto_dispatch )
		pthread_join(s_thread, NULL);

	delete m_factory;
	m_factory = NULL;

	// 销毁ghost工作列表
	m_ghosttasks.clear();

	// 销毁请求列表
	m_desires.lock();
	while(!m_desires.empty()) 
	{
		delete m_desires.front();
		m_desires.pop();
	}
	m_desires.unlock();

	// 销毁等待列表
	for ( taskmap_t::iterator it = m_taskloading.begin(); it != m_taskloading.end(); it++ )
	{
		delete it->second;
	}
	m_taskloading.clear();

	// 销毁observers
	m_observers.lock();
	for (observeridx_t::iterator it = m_observers.begin();
			it != m_observers.end(); it++)
	{
		delete it->second;
	}
	m_observers.clear();
	m_observers.unlock();

	// 销毁cdf index
	m_cdfidx.lock();
	m_cdfidx.clear();
	m_cdfidx.unlock();

	// 确保最后销毁m_cellidx
	m_cellidx.lock();
	for (cellidx_t::iterator it = m_cellidx.begin(); it != m_cellidx.end(); it++)
	{
		delete (*it).second;
	}
	m_cellidx.clear();
	m_cellidx.unlock();
}

const CRegulation& CCells::regulation() const
{
	return m_rule;
}

void CCells::resume()
{
	m_suspend = false;
}

void CCells::suspend()
{
	m_suspend = true;
}

bool CCells::is_suspend()
{
	return m_suspend;
}

void CCells::tick_dispatch(double dt)
{
	static double dump_dt = 0;
	static size_t last_bytes = 0;
	dump_dt += dt;
	if ( dump_dt >= 1.0f )
	{
		size_t tb = m_factory->count_downloadbytes();
		size_t wl = m_factory->count_workload();
		size_t sp = size_t( (tb - last_bytes) / dump_dt ); 
		CLogD("STAT: WL=%d, KB/s=%d, TB=%d\n", wl, sp / 1024, tb);
		dump_dt = 0;
		last_bytes = tb;
	}
	
	if ( m_suspend )
		return;

	bool i_am_busy = false; 

	// 分发结果
	for ( CCell* cell = m_factory->pop_result(); cell != NULL; cell = m_factory->pop_result())
	{
		on_task_finish(cell);
		i_am_busy = true;
	}

	// 分发请求
	m_desires.lock();
	if ( !m_desires.empty() )
	{
		i_am_busy = true;

		size_t max_workload = regulation().worker_thread_num * CELLS_WORKER_MAXWORKLOAD;
		size_t now_workload = m_factory->count_workload();
		size_t max_post_num = now_workload < max_workload ? max_workload - now_workload : 0;

		for( size_t i = 0; !m_desires.empty() && i < max_post_num; i++ )
		{
			CCellTask* task = m_desires.front();
			m_factory->post_work(task->cell(), false);
			m_desires.pop();
			m_taskloading.insert(std::make_pair(task->cell(), task));
		}
	}
	m_desires.unlock();

	// tick ghost, only when i am not busy
	if ( !i_am_busy && regulation().enable_ghost_mode )
	{
		ghost_working();
	}
}

bool CCells::post_desire_cdf(const std::string& name, 
							 int priority, /*= priority_exclusive*/
							 ecdf_loadtype_t cdf_load_type, /*= e_cdf_loadtype_config*/
							 eziptype_t zip_type, /*= e_zip_cdfconfig*/
							 void* user_context, /*= NULL*/
							 CProgressWatcher* watcher /*= NULL*/)
{
	if ( name.empty() ) return false;

	return post_desired(name, e_state_file_cdf, priority, user_context, watcher, zip_type, cdf_load_type) != NULL;
}

bool CCells::post_desire_pkg(const std::string& name, 
	int priority /*= e_priority_exclusive*/, 
	void* user_context /*= NULL*/,
	CProgressWatcher* watcher /*= NULL*/)
{
	if ( name.empty() ) return false;

	return post_desired(name, e_state_file_pkg, priority, user_context, watcher, e_zip_pkg) != NULL;
}

bool CCells::post_desire_file(const std::string& name, 
							  int priority, /*= priority_default*/
							  eziptype_t zip_type, /*= e_zip_cdfconfig*/
							  void* user_context, /*= NULL*/
							  CProgressWatcher* watcher /*= NULL*/)
{
	if ( name.empty() ) return false;

	return post_desired(name, e_state_file_common, priority, user_context, watcher, zip_type) != NULL;
}

void CCells::register_observer(void* target, CFunctorBase* func)
{
	m_observers.lock();
	m_observers.insert(target, func);
	m_observers.unlock();
}

void CCells::remove_observer(void* target)
{
	m_observers.lock();
	observeridx_t::iterator it = m_observers.find(target);
	if ( it != m_observers.end() )
	{
		delete (*it).second;
		m_observers.erase(target);
	}
	m_observers.unlock();
}

void CCells::set_speedfactor(float f)
{
	f = f < 0.0f ? 0.0f : f;
	f = f > 1.0f ? 1.0f : f;
	m_factory->set_speedfactor(f);
}


CCell* CCells::post_desired(const std::string& _name, estatetype_t type, int priority, 
							void* user_context, CProgressWatcher* watcher, 
							eziptype_t zip_type, ecdf_loadtype_t cdf_load_type,
							const std::set<std::string>* cascade_set)
{
	assert(priority <= e_priority_exclusive);
	if ( priority > e_priority_exclusive ) priority = e_priority_exclusive;

	// 处理名字
	std::string name = CUtils::str_trim(_name);
	if ( name.empty() ) return NULL;
	CUtils::str_replace_ch(name, '\\', '/');
	if ( name.find_first_of('/') != 0 )	name = "/" + name;


	CLogD( "post desired: name=%s; type=%d; prio=%d; zipt=%d; loadt=%d\n",
			name.c_str(), type, priority, zip_type, cdf_load_type);

	CCell* cell = NULL;
	m_cellidx.lock();
	cellidx_t::iterator it = m_cellidx.find(name);
	if (it == m_cellidx.end())
	{
		if ( zip_type == e_zip_cdfconfig )
		{
			CLogE("post failed: name=%s; r_ziptype=%d; a free file must specify ziptype!\n", name.c_str(), zip_type);

			m_cellidx.unlock();
			return NULL;
		}
		else if ( m_rule.enable_free_download || type == e_state_file_cdf || type == e_state_file_pkg )
		{
			// desire a new cell
			cell = new CCell(name, "", type);
			m_cellidx.insert(name, cell);
			cell->m_ziptype = zip_type;
		}
		else
		{	
			CLogE("post failed: name=%s; name is not in cdf!\n", name.c_str());

			// desire a non-exist common cell
			m_cellidx.unlock();
			return NULL;
		}
	}
	else
	{
		cell = it->second;

		// request type mismatch
		if ( cell->m_celltype != type )
		{
			CLogE("post failed: name=%s; o_type=%d; r_type=%d; type mismatch!\n", name.c_str(), cell->m_celltype, type);

			m_cellidx.unlock();
			return NULL;
		}
		else if ( zip_type != e_zip_cdfconfig && cell->m_ziptype != zip_type )
		{
			CLogE("post failed: name=%s; o_ziptype=%d; r_ziptype=%d; ziptype mismatch!\n", name.c_str(), cell->m_ziptype, zip_type);

			m_cellidx.unlock();
			return NULL;
		}
	}
	m_cellidx.unlock();

	// set watcher state
	if ( watcher )
	{
		cell->m_watcher = watcher;
		cell->m_watcher->set_step(CProgressWatcher::e_initial);
	}

	// create a task
	CCellTask* task = new CCellTask(cell, priority, type, user_context);

	if ( type == e_state_file_cdf )
	{
		task->cdf_loadtype = cdf_load_type;
		if ( cascade_set )
		{
			task->cdf_cascade_set = *cascade_set;
		}
	}
	

	// auto_dispatch 添加到desire队列
	m_desires.lock();
	m_desires.push(task);
	m_desires.unlock();

	return cell;
}

void CCells::on_task_finish(CCell* cell)
{
	//
	// check cell state
	//

	// success
	if ( cell->m_errorno == e_loaderr_ok )
	{
		CLogD(
			"work down: name=%s; state=%d; errorno=%d; times=%d; \n",
			cell->m_name.c_str(), cell->m_cellstate, cell->m_errorno,
			(int)cell->m_download_times);

		// 处理cdf
		if ( cell->m_celltype == e_state_file_cdf && cell->m_cdf )
		{
			cdf_setupindex(cell);
		}

		// 设置完成校验标记
		cell->m_cellstate = CCell::verified;
	}
	// 下载失败，再做一下尝试
	else if (cell->m_errorno == e_loaderr_download_failed
		&& cell->m_download_times < regulation().remote_urls.size())
	{
		cell->m_cellstate = CCell::unknow;
		cell->m_errorno = e_loaderr_ok;
		m_factory->post_work(cell);
		return;
	}
	// 出错了
	else
	{
		// local only mode hack!
		if ( regulation().only_local_mode && cell->m_errorno == e_loaderr_verify_failed )
		{
			CLogD(
				"local hack done!: name=%s; state=%d; errorno=%d; \n",
				cell->m_name.c_str(), cell->m_cellstate, cell->m_errorno);

			// 处理cdf
			if ( cell->m_celltype == e_state_file_cdf && cell->m_cdf )
			{
				cdf_setupindex(cell);
			}

			cell->m_errorno = e_loaderr_ok;
			cell->m_cellstate = CCell::verified;
		}
		else
		{
			cell->m_cellstate = CCell::error;
		}
	}

	//
	// dispatch tasks
	//
	
	bool all_done_edge_trigger = false; // 用于边界触发
	std::pair<taskmap_t::iterator, taskmap_t::iterator> range
		= m_taskloading.equal_range(cell);
	for ( taskmap_t::iterator it = range.first; it != range.second; it++ )
	{
		all_done_edge_trigger = true;

		CCellTask* task = it->second;
		CCell* cell = task->cell();

		// 处理是否加载cdf内容
		if ( cell->m_celltype == e_state_file_cdf && cell->m_cdf )
		{
			// check cdf index
			m_cdfidx.lock();
			bool can_post = m_cdfidx.find(cell->m_name) != m_cdfidx.end();
			m_cdfidx.unlock();
			if (can_post)
				cdf_postload(task);
		}

		// notify observers
		if ( cell->m_celltype == e_state_file_cdf )
		{
			if ( cell->m_cdf )
			{
				props_list_t ready_props_list;
				props_list_t pending_prop_list;

				ready_props_list.insert(std::make_pair(cell->m_name, &(cell->m_props)));

				for ( celllist_t::iterator sub_it = cell->m_cdf->m_subcells.begin(); sub_it != cell->m_cdf->m_subcells.end(); sub_it++ )
				{
					if ( (*sub_it)->m_cellstate == CCell::verified || (*sub_it)->m_celltype == e_state_file_pkg )
					{
						ready_props_list.insert(std::make_pair((*sub_it)->m_name, &((*sub_it)->m_props)));
					}
					else
					{
						pending_prop_list.insert(std::make_pair((*sub_it)->m_name, &((*sub_it)->m_props)));
					}
				}

				notify_observers(
					cell->m_celltype,
					cell->m_name,
					cell->m_errorno,
					&(cell->m_cdf->m_props),
					&ready_props_list,
					&pending_prop_list,
					task->context());
			}
			else
			{
				notify_observers(
					cell->m_celltype,
					cell->m_name,
					cell->m_errorno,
					NULL,
					NULL,
					NULL,
					task->context());
			}
		}
		else
		{
			notify_observers(
				cell->m_celltype,
				cell->m_name,
				cell->m_errorno,
				&(cell->m_props),
				NULL,
				NULL,
				task->context());
		}

		delete task;
	}

	m_taskloading.erase(range.first, range.second);

	//
	// check state
	//

	if ( all_done_edge_trigger )
	{
		bool all_task_done = true;

		m_desires.lock();
		if ( !m_desires.empty() )
		{
			all_task_done = false;
		}
		m_desires.unlock();

		if ( !m_taskloading.empty() )
		{
			all_task_done = false;
		}

		if ( all_task_done )
		{
			notify_observers(e_state_event_alldone, std::string(""), e_loaderr_ok, NULL, NULL, NULL, NULL);
		}
	}

	// remove watcher
	cell->m_watcher = NULL;
}

void CCells::notify_observers(
	estatetype_t type, const std::string& name, eloaderror_t error_no, 
	const props_t* props, const props_list_t* ready_props, const props_list_t* pending_props,
	void* context)
{
	m_observers.lock();
	for (observeridx_t::iterator it = m_observers.begin();
		it != m_observers.end(); it++)
	{
		(*(it->second))(
			type,
			name,
			error_no,
			props,
			ready_props,
			pending_props,
			context);
	}
	m_observers.unlock();
}

void CCells::cdf_setupindex(CCell* cell)
{
	assert(cell->m_cdf);

	// check if alread setup index
	m_cdfidx.lock();
	if ( m_cdfidx.find(cell->m_name) != m_cdfidx.end() )
	{
		m_cdfidx.unlock();
		return;
	}
	m_cdfidx.unlock();

	for (celllist_t::iterator it =
		cell->m_cdf->m_subcells.begin();
		it != cell->m_cdf->m_subcells.end();)
	{
		CCell* subcell = *it;

		m_cellidx.lock();
		cellidx_t::iterator idxcell_it = m_cellidx.find(
			subcell->m_name);

		if (idxcell_it != m_cellidx.end())
		{
			m_cellidx.unlock();

			// 该名称cell已经存在,以idx中的cell为准
			CCell* idxcell = (*idxcell_it).second;
			assert(idxcell);

			it = cell->m_cdf->m_subcells.erase(it);
			delete subcell;
			subcell = idxcell;
			cell->m_cdf->m_subcells.insert(it, idxcell);
		}
		else
		{
			// 将cell插入idx
			m_cellidx.insert(subcell->m_name, subcell);
			m_cellidx.unlock();

			// insert to ghost task list
			if ( regulation().enable_ghost_mode )
			{
				m_ghosttasks.push_back(subcell);
			}

			it++;
		}
	}

	m_cdfidx.lock();
	m_cdfidx.insert(cell->m_name, cell);
	m_cdfidx.unlock();
}

void CCells::cdf_postload(CCellTask* task)
{
	// index only
	if ( task->cdf_loadtype == e_cdf_loadtype_index )
	{
		return;
	}

	CCell* cell = task->cell();
	assert(cell && cell->m_cdf);

	// mark already load in cascade set
	task->cdf_cascade_set.insert(cell->m_name);
	CLogD("cdf_postload setup cascade index for %s\n", cell->m_name.c_str());

	bool loadall = false;

	if ( task->cdf_loadtype == e_cdf_loadtype_load || task->cdf_loadtype == e_cdf_loadtype_load_cascade )
	{
		loadall = true;
	}

	// load config 'loadall' mark?
	if ( task->cdf_loadtype == e_cdf_loadtype_config )
	{
		props_t::iterator cdf_prop_it =
			cell->m_cdf->m_props.find(CDF_LOADALL);

		if (cdf_prop_it != cell->m_cdf->m_props.end()
			&& CUtils::atoi((*cdf_prop_it).second.c_str()) == 1)
		{
			loadall = true;
		}
	}

	for (celllist_t::iterator it = cell->m_cdf->m_subcells.begin();
		it != cell->m_cdf->m_subcells.end(); it++)
	{
		CCell* subcell = *it;

		// ignore pkg file
		if ( subcell->m_celltype == e_state_file_pkg )
			continue;

		bool postload = loadall;
		// parse config 'load' mark
		if ( !postload && task->cdf_loadtype == e_cdf_loadtype_config )
		{
			props_t::iterator prop_it = subcell->m_props.find(CDF_CELL_LOAD);
			if (prop_it != subcell->m_props.end()
				&& CUtils::atoi((*prop_it).second.c_str()) == 1)
			{
				postload = true;
			}
		}

		if ( subcell->m_celltype == e_state_file_cdf )
		{
			//
			// cdf file
			//
			
			if ( task->cdf_loadtype == e_cdf_loadtype_index_cascade || task->cdf_loadtype == e_cdf_loadtype_load_cascade )
			{
				bool already_load = task->cdf_cascade_set.find(subcell->m_name) != task->cdf_cascade_set.end();

				if ( !already_load )
				{
					post_desired(subcell->m_name, e_state_file_cdf, task->priority(), task->context(), NULL, subcell->m_ziptype, task->cdf_loadtype, &task->cdf_cascade_set);
					CLogD("cdf_postload cdf cascade load %s.\n", subcell->m_name.c_str());
				}
				else
				{
					CLogI("cdf_postload cdf already loaded at prev path %s, ignore this post.\n", subcell->m_name.c_str());
				}

				/** unused
				m_cdfidx.lock();
				bool alreadyindexed = m_cdfidx.find(subcell->m_name) != m_cdfidx.end();
				m_cdfidx.unlock();
				
				if ( !alreadyindexed )
				{
					// 未建立索引，级联调用
					//post_desire_cdf(subcell->m_name, task->priority(), task->cdf_loadtype, subcell->m_ziptype, task->context());
					post_desired(subcell->m_name, e_state_file_cdf, task->priority(), task->context(), subcell->m_ziptype, task->cdf_loadtype, &task->cdf_cascade_set);
				}
				else
				{
					// 已经在索引中，不再级联调用
				}
				*/
			}
			else if ( postload )
			{
				//post_desire_cdf(subcell->m_name, task->priority(), e_cdf_loadtype_index, subcell->m_ziptype, task->context());
				CLogD("cdf_postload file cascade load %s.\n", subcell->m_name.c_str());
				post_desired(subcell->m_name, e_state_file_cdf, task->priority(), task->context(), NULL, subcell->m_ziptype, e_cdf_loadtype_index, &task->cdf_cascade_set);
			}
		}
		else 
		{
			//
			// common file
			//

			if ( postload && subcell->m_cellstate != CCell::verified )
			{
				//post_desire_file(subcell->m_name, task->priority(), subcell->m_ziptype, task->context());
				post_desired(subcell->m_name, e_state_file_common, task->priority(), task->context(), NULL, subcell->m_ziptype);
			}
				
		}
	}

}

void CCells::ghost_working()
{
	size_t task_to_post = CELLS_WORKER_MAXWORKLOAD - m_factory->count_workload();
	task_to_post = task_to_post < 0 ? 0 : task_to_post;

	for ( size_t i = 0; !m_ghosttasks.empty() && i < task_to_post; i++ )
	{
		CCell* cell = m_ghosttasks.front();
		m_ghosttasks.pop_front();
		if ( cell->m_cellstate == CCell::unknow )
		{
			m_factory->post_work(cell, true);
			break;
		}
	}
}

} /* namespace cells */
