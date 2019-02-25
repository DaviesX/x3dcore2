#ifndef THREAD_H_INCLUDED
#define THREAD_H_INCLUDED

#include <queue>
#include <pthread.h>
#include <semaphore.h>

namespace e8util
{

typedef pthread_mutex_t         mutex_t;
typedef unsigned int            tid_t;


class if_task
{
public:
        if_task();
        virtual ~if_task();

        virtual void    run(void* storage) = 0;
        void            assign_worker_id(int worker_id);
        int             worker_id() const;
private:
        int        m_worker_id;
};


class task_info
{
        friend task_info        run(if_task* task);
        friend void             sync(task_info& info);

        friend void*            thread_pool_worker(void* p);
public:
        task_info(tid_t tid, pthread_t thread, if_task* task):
                m_tid(tid), m_thread(thread), m_task(task)
        {
        }

        task_info():
                task_info(0, 0, nullptr)
        {
        }

private:
        tid_t           m_tid;
        pthread_t       m_thread;
        if_task*        m_task;
};


class thread_pool
{
        friend void*    thread_pool_worker(void* p);
public:
        thread_pool(unsigned num_thrs, std::vector<void*> worker_storage = std::vector<void*>());
        ~thread_pool();

        task_info               run(if_task* task);

private:
        sem_t                   m_global_sem;
        pthread_mutex_t         m_global_mutex;
        pthread_mutex_t         m_work_group_mutex;
        pthread_t*              m_workers;
        std::vector<void*>      m_worker_storage;
        unsigned                m_num_thrs;
        std::queue<task_info>   m_tasks;
        bool                    m_is_running = true;

        unsigned                m_uuid = 0;
};

unsigned        cpu_core_count();
mutex_t         mutex();
void            destroy(mutex_t& mutex);
void            lock(mutex_t& mutex);
void            unlock(mutex_t& mutex);
task_info       run(if_task* task);
void            sync(task_info& info);

}

#endif // THREAD_H_INCLUDED

