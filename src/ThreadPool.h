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
Lambda���ʽ������������ʽ���£�

[capture list] (params list) mutable exception-> return type { function body }
������庬������

capture list�������ⲿ�����б�
params list���β��б�
mutableָʾ��������˵���Ƿ�����޸Ĳ���ı���
exception���쳣�趨
return type����������
function body��������
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
 
//lambda���ʽhttps://www.cnblogs.com/DswCnblog/p/5629165.html
// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads)
    :   stop(false)
{
    for(size_t i = 0;i<threads;++i)
        workers.emplace_back(
			[this]//lambda���ʽ
			{
				for (;;)
				{
					//���������tasks��ȡ��һ������ִ��
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock(this->queue_mutex);
						/*
						��ǰ�̵߳���wait()�󽫱�������ֱ������ĳ���̵߳���notify_*���ѵ�ǰ�̣߳�
						���̱߳�����ʱ���ú������Զ�����std::mutex��unlock()�ͷ�����ʹ��������
						�������������ϵ��̵߳��Լ���ִ�С�һ����ǰ�̻߳��֪ͨ(notify��ͨ����
						����ĳ���̵߳���notify_*�����˵�ǰ�߳�)��wait()����Ҳ���Զ�����std::mutex
						��lock()��wait��Ϊ�������������ʹ������ı��������֡�

						�����������������øú���ǰ����ǰ�߳�Ӧ���Ѿ���unique_lock<mutex> lck
						����˼���������ʹ��ͬһ�������������̱߳�����wait������ʹ��ͬһ��unique_lock<mutex>��
						��wait�����ڲ����Զ�����lck.unlock()�Ի�����������ʹ�������������ڻ������ϵ��ָ̻߳�ִ�С�
						ʹ�ñ������������ĵ�ǰ�߳��ڻ��֪ͨ(notified��ͨ������̵߳��� notify_*ϵ�еĺ���)��������
						��wait()�����ָ�ִ�в��Զ�����lck.lock()�Ի�����������

						�������ı�������wait����������ν��(Predicate)��ֻ�е�pred����Ϊfalseʱ���ø�wait�����Ż���
						����ǰ�̣߳��������յ������̵߳�֪ͨ��ֻ�е�predΪtrueʱ�Żᱻ�����������ˣ���Ч��
						while (!pred())  wait(lck).
						*/
						//wait�����������������Զ��ͷ���������enqueue�е�notify_one���Ѻ��Զ�����
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
    {	//�����ŵ����ã���������������
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task](){ (*task)(); });
    }
	//����ĳ��wait�̣߳������ǰû�еȴ��̣߳���ú���ʲôҲ���������ͬʱ���ڶ���ȴ��̣߳�����ĳ���߳��ǲ�ȷ����(unspecified)��
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
