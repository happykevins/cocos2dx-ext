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

#include "CCreationFactory.h"

#include <assert.h>
#include <list>
#include <stdio.h>

#include "CUtils.h"
#include "CCells.h"
#include "CCreationWorker.h"

namespace cells
{

CCreationFactory::CCreationFactory(CCells* host, size_t worker_num) :
		m_host(host), m_worknum(worker_num), m_ghostworker(NULL), m_task_counter(0), m_speedfactor(1.0f)
{
	for (size_t i = 0; i < m_worknum; i++)
	{
		m_workers.push_back(new CCreationWorker(this, i));
	}

	if ( m_host->regulation().enable_ghost_mode )
	{
		m_ghostworker = new CGhostWorker(this, m_worknum + 1);
	}
}

CCreationFactory::~CCreationFactory()
{
	if ( m_ghostworker )
	{
		delete m_ghostworker;
	}

	for (size_t i = 0; i < m_workers.size(); i++)
	{
		delete m_workers[i];
	}
	m_workers.clear();
}

void CCreationFactory::post_work(CCell* cell, bool ghost)
{
	assert(cell);

	CLogD(
			"post work: name=%s; type=%d; errorno=%d; times=%d; \n",
			cell->m_name.c_str(), cell->m_celltype, cell->m_errorno,
			(int)cell->m_download_times);

	//
	// 这里要确保同一时刻只有一个相同cell被加载
	//

	// loading中，不需要投递，等待完成后dispatch即可
	if(cell->m_cellstate == CCell::loading)
	{
		return;
	}
	// 非unknow状态，直接投递finish队列
	else if(cell->m_cellstate != CCell::unknow)
	{
		m_finished.lock();
		m_finished.push(cell);
		m_finished.unlock();
		return;
	}

	// 只在此线程中修改cell状态
	// #issue: 顺序性能否保证？
	cell->m_cellstate = CCell::loading;

	// check if ghost task
	if ( ghost && m_host->regulation().enable_ghost_mode )
	{
		assert(m_ghostworker);
		m_ghostworker->post_work(cell);
	}
	else
	{
		// 根据负载判定选择的worker
		size_t worker_id = 0;
		size_t min_workload = (size_t)-1;
		for (size_t i = 0; i < m_workers.size(); i++)
		{
			if ( m_workers[i]->workload() < min_workload )
			{
				worker_id = i;
				min_workload = m_workers[i]->workload();
			}
		}
		m_workers[worker_id]->post_work(cell);
	}

	m_task_counter++;

	return;
}

CCell* CCreationFactory::pop_result()
{
	CCell* cell = NULL;
	m_finished.lock();
	if (!m_finished.empty())
	{
		cell = m_finished.pop_front();
	}
	m_finished.unlock();
	return cell;
}

void CCreationFactory::notify_work_finished(CCell* cell)
{
	assert(cell);
	m_finished.lock();
	m_finished.push(cell);
	m_finished.unlock();
}

size_t CCreationFactory::count_workload()
{
	size_t sum_workload = 0;
	for (size_t i = 0; i < m_workers.size(); i++)
	{
		sum_workload += m_workers[i]->workload();
	}
	if ( m_ghostworker )
	{
		sum_workload += m_ghostworker->workload();
	}

	return sum_workload;
}

size_t CCreationFactory::count_workingworks()
{
	size_t working_num = 0;
	for (size_t i = 0; i < m_workers.size(); i++)
	{
		if ( m_workers[i]->workload() != 0 )
			working_num++;
	}

	return working_num;
}

size_t CCreationFactory::count_downloadbytes()
{
	size_t sum_bytes = 0;
	for (size_t i = 0; i < m_workers.size(); i++)
	{
		sum_bytes += m_workers[i]->get_downloadbytes();
	}
	if ( m_ghostworker )
	{
		sum_bytes += m_ghostworker->get_downloadbytes();
	}

	return sum_bytes;
}

void CCreationFactory::set_speedfactor(float f)
{
	assert(f >= 0 && f <= 1.0);

	m_speedfactor = f;
}

size_t CCreationFactory::suggest_maxspeed()
{
	// 由于工作线程还负责解压缩和存储到磁盘，不能全速下载，做一个补偿
	float local_factor = 1.8f;

	size_t free_size = m_worknum - count_workingworks();
	free_size = free_size < 1 ? 1 : free_size;

	local_factor = local_factor * free_size / m_worknum;

	return (size_t) (m_host->regulation().max_download_speed * m_speedfactor * local_factor);
}

} /* namespace cells */
