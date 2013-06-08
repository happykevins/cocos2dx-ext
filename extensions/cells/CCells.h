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

#ifndef CCELLS_H_
#define CCELLS_H_

#include "cells.h"
#include "CContainer.h"

//
// TODO List
// TODO: 对url列表的连接进行测速，优先选用最快的url，重试下载的cell是否要调整优先级，以免阻塞后续请求
// TODO: 对加载线程的负载控制，可以启动多线程，但不一定都使用，根据负载来调节，减少libcurl阻塞带来的性能瓶颈，可使用进度回调，判断待下载字节数来控制
// TODO: 可以只verify，在下载前知道要下载的文件及大小
//
namespace cells
{

class CCell;
class CCreationFactory;

/*
* CCellTask - desire task
* 	1. 记载一次需求任务的信息
* 	2. 每一次post都会产生该对象,task的生存周期如下：
*	  [dispatch线程]cells::post_desire-->[dispatch线程]factory::post_work-->[worker线程]worker::do_work
*	  -->[worker线程]factory::notify_work_finish-->[dispatch线程]cells::dispatch_result->cells::on_task_finish
*/
class CCellTask
{
private:
	CCell*	m_cell;
	int		m_priority;
	estatetype_t m_type;
	void*	m_context;

public:
	ecdf_loadtype_t cdf_loadtype;
	std::set<std::string> cdf_cascade_set;	// 检查cdf循环引用

	inline CCell* cell() { return m_cell; }
	inline int priority() { return m_priority; }
	inline estatetype_t type() { return m_type; }
	inline void* context() { return m_context;}

	CCellTask(CCell* _cell, int _priority, estatetype_t _type, void* _user_context = NULL) : 
	m_cell(_cell), m_priority(_priority), m_type(_type), m_context(_user_context), cdf_loadtype(e_cdf_loadtype_config) {}

	// 用于按照priority的排序
	struct less_t : public std::binary_function<CCellTask*, CCellTask*, bool>
	{
		bool operator()(CCellTask*& __x, CCellTask*& __y) const
		{ return __x->priority() < __y->priority(); }
	};
};

typedef CMap<void*, CFunctorBase*> observeridx_t;
typedef CPriorityQueue<CCellTask*, CCellTask::less_t> desiresque_t;
typedef CMap<std::string, class CCell*> cellidx_t;
typedef CMap<std::string, class CCell*> cdfidx_t;


/*
 * CCells
 * 	cells系统的接口类
 */
class CCells : public CellsHandler
{
	typedef std::multimap<CCell*, CCellTask*> taskmap_t;
public:
	// @see CellsHandler
	virtual const CRegulation& regulation() const;

	// @see CellsHandler
	virtual void tick_dispatch(double dt);

	// @see CellsHandler
	virtual void resume();

	// @see CellsHandler
	virtual void suspend();

	// @see CellsHandler
	virtual bool is_suspend();

	// @see CellsHandler
	virtual bool post_desire_cdf(const std::string& name, 
		int priority = e_priority_exclusive, 
		ecdf_loadtype_t cdf_load_type = e_cdf_loadtype_config,
		eziptype_t zip_type = e_zip_cdfconfig,
		void* user_context = NULL,
		CProgressWatcher* watcher = NULL);

	// @see CellsHandler
	virtual bool post_desire_pkg(const std::string& name, 
		int priority = e_priority_exclusive, 
		void* user_context = NULL,
		CProgressWatcher* watcher = NULL);

	// @see CellsHandler
	virtual bool post_desire_file(const std::string& name, 
		int priority = e_priority_default,
		eziptype_t zip_type = e_zip_cdfconfig,
		void* user_context = NULL,
		CProgressWatcher* watcher = NULL);

	// @see CellsHandler
	virtual void register_observer(void* target, CFunctorBase* func);

	// @see CellsHandler
	virtual void remove_observer(void* target);

	// @see CellsHandler
	virtual void set_speedfactor(float f);

	// constructor
	CCells();
	bool initialize(const CRegulation& rule);
	void destroy();

protected:
	CCell* post_desired(
		const std::string& _name, estatetype_t type, 
		int priority, 
		void* user_context = NULL, 
		CProgressWatcher* watcher = NULL,
		eziptype_t zip_type = e_zip_cdfconfig,
		ecdf_loadtype_t cdf_load_type = e_cdf_loadtype_config,
		const std::set<std::string>* cascade_set = NULL);
	
private:
	//
	// 以下函数只在dispatch线程中调用
	//
	static void* cells_working(void* context);
	void on_task_finish(CCell* cell);
	void cdf_setupindex(CCell* cell);
	void cdf_postload(CCellTask* task);
	void ghost_working();
	void notify_observers(
		estatetype_t type, const std::string& name, eloaderror_t error_no, 
		const props_t* props, const props_list_t* ready_props, const props_list_t* pending_props,
		void* context);

protected:
	CRegulation 		m_rule;
	CCreationFactory* 	m_factory;
	volatile bool 		m_suspend;
	cellidx_t 			m_cellidx;
	cdfidx_t			m_cdfidx;		// cdf建立索引状态表
	observeridx_t		m_observers;
	desiresque_t		m_desires;
	taskmap_t			m_taskloading;	// 正在loading的task表
	// ghost工作列表
	std::list<class CCell*> m_ghosttasks;

	friend class CCreationFactory;
};

} /* namespace cells */
#endif /* CCELLS_H_ */
