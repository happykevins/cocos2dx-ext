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

#ifndef CCONTAINER_H_
#define CCONTAINER_H_

#include <pthread.h>
#include <semaphore.h>

#include <algorithm>
#include <queue>
#include <stack>
#include <map>
#include <set>

namespace cells
{

/*
 * 支持互斥量接口
 */
class CMutexLockable
{
public:
	CMutexLockable()
	{
		pthread_mutex_init(&m_mutex, NULL);
	}
	virtual ~CMutexLockable()
	{
		pthread_mutex_destroy(&m_mutex);
	}
	inline void lock()
	{
		pthread_mutex_lock(&m_mutex);
	}

	inline void unlock()
	{
		pthread_mutex_unlock(&m_mutex);
	}

	inline pthread_mutex_t* mutex()
	{
		return &m_mutex;
	}

protected:
	pthread_mutex_t m_mutex;
};

/*
 * CQueue
 */
template<typename T>
class CQueue : public CMutexLockable
{
public:
	typedef std::queue<T> _queue_t;
	typedef typename _queue_t::value_type value_type;
	typedef typename _queue_t::reference reference;

	inline bool empty()
	{
		return m_queue.empty();
	}

	inline size_t size()
	{
		return m_queue.size();
	}

	inline void push(const value_type& v)
	{
		m_queue.push(v);
	}

	inline reference pop_front()
	{
		reference ref = m_queue.front();
		m_queue.pop();
		return ref;
	}

protected:
	_queue_t m_queue;
};

/*
 * CQueue
 * 	1.stl的版本只返回const类型，不适用。
 * 	2.注意使用安全，请勿轻易修改排序所用的字段。
 */
template<typename T, typename Comp>
class CPriorityQueue : public CMutexLockable
{
public:
	typedef std::vector<T> _queue_t;
	typedef typename _queue_t::value_type value_type;
	typedef typename _queue_t::reference reference;

	inline bool empty()
	{
		return m_queue.empty();
	}

	inline size_t size()
	{
		return m_queue.size();
	}

	inline void push(const value_type& v)
	{
		m_queue.push_back(v);
		std::push_heap(m_queue.begin(), m_queue.end(), m_comp);
	}

	inline reference front()
	{
		return m_queue.front();
	}

	inline void pop()
	{
		std::pop_heap(m_queue.begin(), m_queue.end(), m_comp);
		m_queue.pop_back();
	}

protected:
	_queue_t m_queue;
	Comp m_comp;
};

template<typename K, typename V>
class CMap : public CMutexLockable
{
public:
	typedef std::map<K, V> _map_t;
	typedef typename _map_t::key_type 			key_type;
	typedef typename _map_t::mapped_type 		mapped_type;
	typedef typename _map_t::value_type 		value_type;
	typedef typename _map_t::pointer         	pointer;
	typedef typename _map_t::const_pointer   	const_pointer;
	typedef typename _map_t::reference       	reference;
	typedef typename _map_t::const_reference 	const_reference;
	typedef typename _map_t::iterator			iterator;
	typedef typename _map_t::const_iterator	const_iterator;

	std::pair<iterator, bool> insert(const key_type& key, mapped_type& v)
	{
		return m_map.insert(std::make_pair(key, v));
	}

	iterator find(const key_type& key)
	{
		return m_map.find(key);
	}

	void erase(const key_type& key)
	{
		m_map.erase(key);
	}

	void erase(iterator i)
	{
		m_map.erase(i);
	}

	iterator begin()
	{
		return m_map.begin();
	}

	iterator end()
	{
		return m_map.end();
	}

	bool empty() const
	{
		return m_map.empty();
	}

	size_t size() const
	{
		return m_map.size();
	}

	void clear()
	{
		m_map.clear();
	}

protected:
	_map_t m_map;
};


class CMutexScopeLocker
{
public:
	inline CMutexScopeLocker(pthread_mutex_t* mutex) :
			m_mutex(mutex)
	{
		pthread_mutex_lock(m_mutex);
	}
	inline ~CMutexScopeLocker()
	{
		pthread_mutex_unlock(m_mutex);
	}

private:
	pthread_mutex_t* m_mutex;
};

#define CMutexScopeLock(m) CMutexScopeLocker __lock(m);


} /* namespace cells */
#endif /* CCONTAINER_H_ */
