#include <thread>
#include "thread.h"


e8util::if_task_storage::if_task_storage()
{
}

e8util::if_task_storage::~if_task_storage()
{
}

e8util::if_task::if_task(bool drop_on_completion):
        m_drop_on_completion(drop_on_completion)
{
}

e8util::if_task::~if_task()
{
}

bool
e8util::if_task::is_drop_on_completion() const
{
        return m_drop_on_completion;
}

void
e8util::if_task::assign_worker_id(int worker_id)
{
        m_worker_id = worker_id;
}

int
e8util::if_task::worker_id() const
{
        return m_worker_id;
}


e8util::task_info::task_info(tid_t tid, pthread_t thread, if_task* task):
        m_tid(tid), m_thread(thread), m_task(task)
{
}

e8util::task_info::task_info():
        task_info(0, 0, nullptr)
{
}

e8util::if_task*
e8util::task_info::task() const
{
        return m_task;
}


unsigned
e8util::cpu_core_count()
{
        return std::thread::hardware_concurrency();
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

struct thread_worker_data
{
        thread_worker_data(e8util::if_task* task,
                           e8util::if_task_storage* task_data):
                task(task),
                task_data(task_data)
        {}

        ~thread_worker_data()
        {
        }

        e8util::if_task*                task;
        e8util::if_task_storage*        task_data;
};

static void*
worker(void* p)
{
        thread_worker_data* data = static_cast<thread_worker_data*>(p);

        data->task->run(nullptr);
        delete data;

        pthread_exit(nullptr);
        return nullptr;
}

e8util::task_info
e8util::run(if_task* task, if_task_storage* task_data)
{
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        task_info info(0, 0, task);

        task->assign_worker_id(-1);

        thread_worker_data* thrdata = new thread_worker_data(task, task_data);
        pthread_create(&info.m_thread, &attr, worker, thrdata);
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
        uint32_t        reserved0;
};

void*   thread_pool_worker(void* p);
}

void*
e8util::thread_pool_worker(void* p)
{
        e8util::thread_pool* this_ = static_cast<e8util::thread_pool_worker_data*>(p)->this_;
        unsigned worker_id = static_cast<e8util::thread_pool_worker_data*>(p)->worker_id;

        do {
                sem_wait(&this_->m_enter_sem);
                while (true) {
                        pthread_mutex_lock(&this_->m_enter_mutex);
                        if (!this_->m_tasks.empty()) {
                                // Retrieve task.
                                e8util::task_info info = this_->m_tasks.front();
                                this_->m_tasks.pop();

                                pthread_mutex_unlock(&this_->m_enter_mutex);

                                info.m_task->assign_worker_id(static_cast<int>(worker_id));
                                info.m_task->run(this_->m_worker_storage[worker_id]);

                                if (!info.m_task->is_drop_on_completion()) {
                                        pthread_mutex_lock(&this_->m_exit_mutex);

                                        this_->m_completed_tasks.push(info);

                                        pthread_mutex_unlock(&this_->m_exit_mutex);
                                        sem_post(&this_->m_exit_sem);
                                }
                        } else {
                                pthread_mutex_unlock(&this_->m_enter_mutex);
                                break;
                        }
                }
        } while (this_->m_is_running);

        delete static_cast<e8util::thread_pool_worker_data*>(p);
        pthread_exit(nullptr);
        return nullptr;
}

e8util::thread_pool::thread_pool(unsigned num_thrs,
                                 std::vector<if_task_storage*> worker_storage):
        m_num_thrs(num_thrs)
{
        sem_init(&m_enter_sem, 0, 0);
        sem_init(&m_exit_sem, 0, 0);
        pthread_mutex_init(&m_enter_mutex, nullptr);
        pthread_mutex_init(&m_exit_mutex, nullptr);
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
                sem_post(&m_enter_sem);

        for (unsigned i = 0; i < m_num_thrs; i ++)
                pthread_join(m_workers[i], nullptr);

        delete [] m_workers;
        m_num_thrs = 0;

        e8util::destroy(m_enter_mutex);
        e8util::destroy(m_exit_mutex);
        e8util::destroy(m_work_group_mutex);
}

e8util::task_info
e8util::thread_pool::run(if_task* t)
{
        pthread_mutex_lock(&m_enter_mutex);

        task_info info(m_uuid ++, 0, t);
        m_tasks.push(info);

        pthread_mutex_unlock(&m_enter_mutex);
        sem_post(&m_enter_sem);

        return info;
}

e8util::task_info
e8util::thread_pool::retrieve_next_completed()
{
        e8util::task_info info;

        sem_wait(&m_exit_sem);
        pthread_mutex_lock(&m_exit_mutex);

        info = m_completed_tasks.front();
        m_completed_tasks.pop();

        pthread_mutex_unlock(&m_exit_mutex);

        return info;
}
