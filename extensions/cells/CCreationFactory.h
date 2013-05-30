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

#ifndef CCREATIONFACTORY_H_
#define CCREATIONFACTORY_H_

#include <cstddef>
#include <vector>

#include "CContainer.h"

namespace cells
{

class CCells;
class CCell;
class CCellTask;
class CCreationWorker;

/*
 * 创建cell的工厂类
 * 	1.多工作线程
 * 	2.任务分派，负载平衡
 * 	3.下载速度控制，拥塞控制
 * 	4.回调及事件通知
 * 	5.*非线程安全
 *	6.*确保对于每一个cell，从post_work到dispatch_result结束过程中cellstate都处于loading状态,只有loading状态，才允许修改cell内容
 */
class CCreationFactory
{
public:
	CCreationFactory(CCells* host, size_t worker_num);
	virtual ~CCreationFactory();

	// 投递一个任务
	void post_work(CCell* cell, bool ghost = false);

	// 获得一个任务结果，如果没有返回NULL
	CCell* pop_result();

	// 获得负载
	size_t count_workload();

	// 获得工作worker的数量
	size_t count_workingworks();

	// 获得当前下载的字节数
	size_t count_downloadbytes();

	// 设置下载速度控制系数 0.0~1.0
	void set_speedfactor(float f); 

protected:
	// worker thread callback
	void notify_work_finished(CCell* cell);
	// worker thread require speed suggestion
	size_t suggest_maxspeed();

private:
	CCells* m_host;
	const size_t m_worknum;
	std::vector<CCreationWorker*> m_workers;
	CCreationWorker* m_ghostworker;
	size_t m_task_counter; // 处理过的任务计数器
	volatile float m_speedfactor;

	CQueue<class CCell*> m_finished;

	friend class CCreationWorker;
	friend class CGhostWorker;
};

} /* namespace cells */
#endif /* CCREATIONFACTORY_H_ */
