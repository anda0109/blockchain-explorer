#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
/*
Lambda表达式完整的声明格式如下：

[capture list] (params list) mutable exception-> return type { function body }
各项具体含义如下

capture list：捕获外部变量列表
params list：形参列表
mutable指示符：用来说用是否可以修改捕获的变量
exception：异常设定
return type：返回类型
function body：函数体
*/

class ThreadPool {
public:
    ThreadPool(size_t);
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
	int task_wait() { return tasks.size(); }
    ~ThreadPool();
private:
    // need to keep track of threads so we can join them
    std::vector< std::thread > workers;
    // the task queue
    std::queue< std::function<void()> > tasks;
    
    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};
 
//lambda表达式https://www.cnblogs.com/DswCnblog/p/5629165.html
// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads)
    :   stop(false)
{
    for(size_t i = 0;i<threads;++i)
        workers.emplace_back(
			[this]//lambda表达式
			{
				for (;;)
				{
					//从任务队列tasks中取出一个任务并执行
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock(this->queue_mutex);
						/*
						当前线程调用wait()后将被阻塞，直到另外某个线程调用notify_*唤醒当前线程；
						当线程被阻塞时，该函数会自动调用std::mutex的unlock()释放锁，使得其它被
						阻塞在锁竞争上的线程得以继续执行。一旦当前线程获得通知(notify，通常是
						另外某个线程调用notify_*唤醒了当前线程)，wait()函数也是自动调用std::mutex
						的lock()。wait分为无条件被阻塞和带条件的被阻塞两种。

						无条件被阻塞：调用该函数前，当前线程应该已经对unique_lock<mutex> lck
						完成了加锁。所有使用同一个条件变量的线程必须在wait函数中使用同一个unique_lock<mutex>。
						该wait函数内部会自动调用lck.unlock()对互斥锁解锁，使得其他被阻塞在互斥锁上的线程恢复执行。
						使用本函数被阻塞的当前线程在获得通知(notified，通过别的线程调用 notify_*系列的函数)而被唤醒
						后，wait()函数恢复执行并自动调用lck.lock()对互斥锁加锁。

						带条件的被阻塞：wait函数设置了谓词(Predicate)，只有当pred条件为false时调用该wait函数才会阻
						塞当前线程，并且在收到其它线程的通知后只有当pred为true时才会被解除阻塞。因此，等效于
						while (!pred())  wait(lck).
						*/
						//wait进入阻塞，阻塞后自动释放锁；当被enqueue中的notify_one唤醒后自动加锁
						this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
						if (this->stop && this->tasks.empty())
							return;
						task = std::move(this->tasks.front());
						this->tasks.pop();
					}

					task();
				}
			}
        );
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
    std::future<return_type> res = task->get_future();
    {	//大括号的作用，限制锁的作用域
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task](){ (*task)(); });
    }
	//唤醒某个wait线程，如果当前没有等待线程，则该函数什么也不做；如果同时存在多个等待线程，则唤醒某个线程是不确定的(unspecified)。
    condition.notify_one();
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(std::thread &worker: workers)
        worker.join();
}

#endif
