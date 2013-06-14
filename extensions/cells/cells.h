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

#ifndef CELLS_H_
#define CELLS_H_

#include <vector>
#include <list>
#include <map>
#include <string>

#define CELLS_DEFAULT_WORKERNUM			4
#define CELLS_WORKER_MAXWORKLOAD		2
#define CELLS_DOWNLOAD_SPEED_NOLIMIT	(1024 * 1024 * 100) // 100MB
#define CELLS_GHOST_DOWNLOAD_SPEED		(1024 * 32)			// 32KB
#define CELLS_REMOTE_ZIPFILE_SUFFIX		""
#define CELLS_DEFAULT_TEMP_SUFFIX		".temp"
#define CELLS_DEFAULT_HASH_SUFFIX		".hash"

namespace cells
{

//
// 内建属性名
//
extern const char* CDF_VERSION;			//= "version"	string
extern const char* CDF_LOADALL;			//= "loadall"	boolean:		0 | 1
extern const char* CDF_CELL_CDF;		//= "cdf"		boolean: 		0 | 1 : is 'e_celltype_cdf' type
extern const char* CDF_CELL_NAME;		//= "name"		string
extern const char* CDF_CELL_LOAD;		//= "load"		boolean:		0 | 1
extern const char* CDF_CELL_HASH;		//= "hash"		string
extern const char* CDF_CELL_SIZE;		//= "size"		int
extern const char* CDF_CELL_ZHASH;		//= "zhash"		string
extern const char* CDF_CELL_ZSIZE;		//= "zsize"		int
extern const char* CDF_CELL_ZIP;		//=	"zip"		int				0 - nozip | 1 - zlib
extern const char* CDF_TAG_PKG;			//= "pkg"
extern const char* CDF_TAG_CELL;		//= "cell"

// 属性表
typedef std::map<std::string, std::string> props_t; 
// 属性表列表
typedef std::map<std::string, props_t*> props_list_t;

// 压缩类型
enum eziptype_t
{
	e_zip_cdfconfig = -1,	// 由CDF中的描述决定
	e_zip_none = 0,			// 未压缩
	e_zip_zlib,				// 使用zlib压缩
	e_zip_pkg,				// zip package
};

// state类型
enum estatetype_t
{
	e_state_file_common = 0,	// 普通文件
	e_state_file_cdf = 1,		// CDF文件
	e_state_file_pkg = 2,		// PKG文件
	e_state_event_alldone = 10	// callback事件：代表请求的任务全部完成
};

// 优先级
enum epriority_t {
	e_priority_default 		= 0,
	e_priority_exclusive 	= (unsigned short)-1, // 最大65535
};

// CDF文件加载方式
enum ecdf_loadtype_t
{
	e_cdf_loadtype_config = 0,		// 建立此cdf索引，并按配置来进行加载操作
	e_cdf_loadtype_index,			// 只建立此cdf的索引，不加载文件
	e_cdf_loadtype_index_cascade,	// 级联建立子cdf索引，不加载common文件
	e_cdf_loadtype_load,			// 建立索引并加载此cdf描述的文件
	e_cdf_loadtype_load_cascade,	// 建立索引并级联加载所有子cdf描述的文件
};

// 加载错误类型
enum eloaderror_t
{
	e_loaderr_ok = 0, 
	e_loaderr_openfile_failed, 
	e_loaderr_download_failed, 
	e_loaderr_decompress_failed, 
	e_loaderr_verify_failed, 
	e_loaderr_patchup_failed
};


//
// cells系统规则定制
//
struct CRegulation
{
	CRegulation();

	std::vector<std::string> remote_urls;// 下载路径列表 (Trick:由于在一个url失败会按顺序尝试下面的url，因此可以添加多个相同的url，以实现尝试次数的控制
	std::string local_url;				// 本地存储路径

	size_t worker_thread_num;			// 工作线程数
	size_t max_download_speed;			// 下载速度上限
	bool auto_dispatch;					// 是否启动自动派发线程
	bool only_local_mode;				// 是否开启本地模式：本地文件不匹配也不进行download操作

	bool enable_ghost_mode;				// 是否开启ghost模式：(默认关闭)
	size_t max_ghost_download_speed;	// ghost的下载速度
	bool enable_free_download;			// 是否开启自由下载模式：(默认关闭)，开启此模式可以自由需求cdf没有描述过的文件

	std::string remote_zipfile_suffix;	// remote端zip文件后缀
	std::string tempfile_suffix;		// 临时下载文件后缀
	std::string temphash_suffix;		// 临时hash文件后缀
};

//
// CFunctorBase-封装函数闭包
//
class CFunctorBase
{
public:
	virtual void operator() (
		estatetype_t type, const std::string& name, eloaderror_t error_no, 
		const props_t* props, const props_list_t* ready_props, const props_list_t* pending_props,
		void* context) = 0;
};

class CFunctorG : public CFunctorBase
{
public:
	typedef void (*cb_func_g_t)(estatetype_t type, const std::string& name, eloaderror_t error_no, const props_t* props, const props_list_t* ready_props, const props_list_t* pending_props, void* context);
	CFunctorG(cb_func_g_t cb_func) : m_cb_func(cb_func) {}
	CFunctorG(const CFunctorG& other) : m_cb_func(other.m_cb_func) {}
	CFunctorG() : m_cb_func(NULL) {}
	virtual ~CFunctorG(){ m_cb_func = NULL; }
	virtual void operator() (estatetype_t type, const std::string& name, eloaderror_t error_no, const props_t* props, const props_list_t* ready_props, const props_list_t* pending_props, void* context)
	{
		m_cb_func(type, name, error_no, props, ready_props, pending_props, context);
	}

protected:
	cb_func_g_t m_cb_func;
};

template<typename T>
class CFunctorM : public CFunctorBase
{
public:
	typedef void (T::*mfunc_t)(estatetype_t type, const std::string& name, eloaderror_t error_no, const props_t* props, const props_list_t* ready_props, const props_list_t* pending_props, void* context);
	CFunctorM(T* _t, mfunc_t _f) : m_target(_t), m_func(_f) {}
	CFunctorM(const CFunctorM<T>& other) : m_target(other.m_target), m_func(other.m_func) {}
	void operator=(const CFunctorM<T>& other)
	{
		m_target = other.m_target;
		m_func = other.m_func;
	}

	void operator() (estatetype_t type, const std::string& name, eloaderror_t error_no, const props_t* props, const props_list_t* ready_props, const props_list_t* pending_props, void* context)
	{
		(m_target->*m_func)(type, name, error_no, props, ready_props, pending_props, context);
	}

protected:
	T* m_target;
	mfunc_t m_func;
};

template<typename F>
CFunctorG* make_functor_g(F& _f)
{
	return new CFunctorG(_f);
}
template<typename T>
CFunctorM<T>* make_functor_m(T* _t, typename CFunctorM<T>::mfunc_t _f)
{
	return new CFunctorM<T>(_t, _f);
}

struct CProgressWatcher
{
	enum estep_t
	{
		e_initial,
		e_verify_local,
		e_download,
		e_unzip,
		e_verify_download,
		e_finish,
		e_error
	};

	volatile estep_t step;
	volatile double now;	// progress now
	volatile double total;	// progress total

	float progress(); // format: 100.00%

	void set_step(estep_t _step);
	CProgressWatcher();
};

//
// cells系统接口
//
class CellsHandler
{
public:
	virtual ~CellsHandler() {}

	/*
	* 返回cells系统配置规则
	*/
	virtual const CRegulation& regulation() const = 0;

	/*
	* 派发结果通告
	*	@param - dt : 间隔时间(秒)
	* 	1.auto_dispatch设置为false，用户通过调用该方法来派发执行结果
	* 		以确保回调函数在用户线程中执行
	* 	2.auto_dispatch为true,cells系统线程将派
	* 		发回调，用户需要对回调函数的线程安全性负责
	*/
	virtual void tick_dispatch(double dt) = 0;

	/*
	* 恢复执行
	* 	1.与suspend对应
	*/
	virtual void resume() = 0;

	/*
	* 挂起cells系统：暂停cells系统的所有活动
	* 	1.挂起状态可以提交需求，但是不会被执行
	* 	2.在挂起前已经在执行中的任务会继续执行
	*/
	virtual void suspend() = 0;

	/*
	* 判断cells系统是否处于挂起状态
	*/
	virtual bool is_suspend() = 0;

	/*
	* 需求一个cdf文件
	* 	1.cdf - cells description file
	* 	2.包含了对用户文件列表的描述
	* 	@param name - cdf文件名
	* 	@param priority - 优先级,此值越高，越会优先处理; priority_exclusive代表抢占模式
	*	@param cdf_load_type - 加载完cdf文件后，如何处理子文件
	*	@param zip_type - 请求文件的压缩类型，默认由CDF配置决定
	*	@param user_context - 回调时传递给observer的user_context
	*	@param watcher - 加载状态监控器
	*	@return - 是否成功：name语法问题会导致失败
	*/
	virtual bool post_desire_cdf(const std::string& name, 
		int priority = e_priority_exclusive, 
		ecdf_loadtype_t cdf_load_type = e_cdf_loadtype_config,
		eziptype_t zip_type = e_zip_cdfconfig,
		void* user_context = NULL,
		CProgressWatcher* watcher = NULL) = 0;

	/*
	* 需求一个package文件
	* 	1.pkg - a package is a zip file contains serious files
	* 	@param name - 文件名
	* 	@param priority - 优先级,此值越高，越会优先处理; priority_exclusive代表抢占模式
	*	@param user_context - 回调时传递给observer的user_context
	*	@param watcher - 加载状态监控器
	*	@return - 是否成功：name语法问题会导致失败
	*/
	virtual bool post_desire_pkg(const std::string& name, 
		int priority = e_priority_exclusive, 
		void* user_context = NULL,
		CProgressWatcher* watcher = NULL) = 0;

	/*
	* 需求一个用户文件
	* 	1.需求的文件如果没有包含在此前加载的cdf之中，会因为无法获得文件hash导致每次都需要下载
	* 	@param name - 文件名
	* 	@param priority - 优先级,此值越高，越会优先处理; priority_exclusive代表抢占模式
	*	@param zip_type - 请求文件的压缩类型，默认由CDF配置决定
	*	@param user_context - 回调时传递给observer的user_context
	*	@param watcher - 加载状态监控器
	*	@return - 是否成功：如果没有开启free_download，如果需求之前加载的cdf表中没有包含的文件，会导致返回失败
	*/
	virtual bool post_desire_file(const std::string& name, 
		int priority = e_priority_default,
		eziptype_t zip_type = e_zip_cdfconfig,
		void* user_context = NULL,
		CProgressWatcher* watcher = NULL) = 0;

	/*
	* 注册监听器，事件完成会收到通知
	* 	1.注意在目标target失效前要移除监听器，否则会出现内存访问失败问题
	* 	@param target - 目标对象
	* 	@param func - 目标对象的回调方法 - 用make_functor函数获得
	* 		回调方法原型 - void (T::*mfunc_t)(const char* name, int type, int error_no);
	*/
	virtual void register_observer(void* target, CFunctorBase* func) = 0;

	/*
	* 移除监听器
	* 	@param target - 注册的目标对象
	*/
	virtual void remove_observer(void* target) = 0;

	/*
	* 设置下载速度系数，用于边玩边下载控制
	* 	@param f - 下载速度系数[0~1]: 0取消限速
	*/
	virtual void set_speedfactor(float f) = 0;
};

// 创建cells接口，失败返回NULL
CellsHandler* cells_create(const CRegulation& rule);
// 销毁cells接口
void cells_destroy(CellsHandler* handler);

}/* namespace cells */
#endif /* CELLS_H_ */