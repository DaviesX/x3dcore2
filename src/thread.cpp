#include <sys/sysinfo.h>
#include "thread.h"


unsigned
e8util::cpu_core_count()
{
        return get_nprocs();
}

e8util::mutex_t
e8util::mutex()
{
        mutex_t mutex;
        pthread_mutex_init(&mutex, nullptr);
        return mutex;
}

void
e8util::destroy(mutex_t& mutex)
{
        pthread_mutex_destroy(&mutex);
}

void
e8util::lock(mutex_t& mutex)
{
        pthread_mutex_lock(&mutex);
}

void
e8util::unlock(mutex_t& mutex)
{
        pthread_mutex_unlock(&mutex);
}

static void*
worker(void* p)
{
        e8util::if_task* task = static_cast<e8util::if_task*>(p);
        task->main(nullptr);
        pthread_exit(nullptr);
}

e8util::task_info
e8util::run(if_task* task)
{
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        task_info info(0, -1, task);
        pthread_create(&info.m_thread, &attr, worker, task);
        return info;
}

void
e8util::sync(task_info& info)
{
        pthread_join(info.m_thread, nullptr);
}

namespace e8util
{
struct thread_pool_worker_data
{
        thread_pool_worker_data(thread_pool* this_, unsigned worker_id):
                this_(this_), worker_id(worker_id)
        {
        }

        thread_pool*    this_;
        unsigned        worker_id;
};

void*   thread_pool_worker(void* p);
}

void*
e8util::thread_pool_worker(void* p)
{
        e8util::thread_pool* this_ = static_cast<e8util::thread_pool_worker_data*>(p)->this_;
        unsigned worker_id = static_cast<e8util::thread_pool_worker_data*>(p)->worker_id;

        do {
                sem_wait(&this_->m_global_sem);
                while (true) {
                        pthread_mutex_lock(&this_->m_global_mutex);
                        if (!this_->m_tasks.empty()) {
                                // Retrieve task.
                                e8util::task_info info = this_->m_tasks.front();
                                this_->m_tasks.pop();

                                pthread_mutex_unlock(&this_->m_global_mutex);

                                info.m_task->main(this_->m_worker_storage[worker_id]);
                        } else {
                                pthread_mutex_unlock(&this_->m_global_mutex);
                                break;
                        }
                }
        } while (this_->m_is_running);

        delete static_cast<e8util::thread_pool_worker_data*>(p);
        pthread_exit(nullptr);
}

e8util::thread_pool::thread_pool(unsigned num_thrs, std::vector<void*> worker_storage):
        m_num_thrs(num_thrs)
{
        sem_init(&m_global_sem, 0, 0);
        pthread_mutex_init(&m_global_mutex, nullptr);
        pthread_mutex_init(&m_work_group_mutex, nullptr);

        m_workers = new pthread_t [num_thrs];
        m_worker_storage = worker_storage;

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        for (unsigned i = 0; i < num_thrs; i ++)
                pthread_create(&m_workers[i], &attr, thread_pool_worker, new thread_pool_worker_data(this, i));
}

e8util::thread_pool::~thread_pool()
{
        m_is_running = false;
        for (unsigned i = 0; i < m_num_thrs; i ++)
                sem_post(&m_global_sem);

        for (unsigned i = 0; i < m_num_thrs; i ++)
                pthread_join(m_workers[i], nullptr);

        delete [] m_workers;
        m_num_thrs = 0;

        e8util::destroy(m_global_mutex);
        e8util::destroy(m_work_group_mutex);
}

e8util::task_info
e8util::thread_pool::run(if_task* t)
{
        pthread_mutex_lock(&m_global_mutex);

        task_info info(m_uuid ++, -1, t);
        m_tasks.push(info);

        pthread_mutex_unlock(&m_global_mutex);
        sem_post(&m_global_sem);

        return info;
}
