.. _parpool:

Parallel Worker Pool
=====================

The Pool Class
##################

.. jsl-class:: JSL::Async::Pool
   :file: Async/ParallelPool.h


Distribution Policies
////////////////////////////

When the user calls the *LoopTask* (or the *LoopReturn*) function, they have a choice in how the tasks in the rage [0,nLoop) are distributed to the N workers. This is determined by the DistributionPolicy which is passed:

.. doxygenenum:: JSL::Async::DistributionPolicy
    

Sequential Distribution
--------------------------

This policy can also be thought of as 'block distribution': the range [0,nLoop) is broken up into N chunks: [0,N), [N,2N) (etc.), and each worker is given those chunks up front.
 
This allows the compiler to perform optimisations such as loop unrolling, and is therefore the best suited distribution when nLoop is very large, and each individual task is quick to compute. 

If, however, the tasks have strongly varying execution times (i.e. tasks for n > C take much longer than for n < C), then this may be highly inefficient, as the quick-executing chunks will complete, and the parallel model will degrade into a single-threaded instance acting upon the slow-executing block.

Balanced Distribution
-----------------------
 
This is the 'pick one, move along' policy. Each worker takes the next loop task from the queue, and completes it, before dipping back into the queue. 

This induces slightly higher overhead than the Sequential distribution (1 atomic operation per task, and no loop unrolling), but ensures that no worker is ever over-burdened by multiple tasks. In the case of a smaller number of heavier tasks, this will likely be the optimal distribution policy.
 
Usage
################



.. code-block:: cpp

   #include <JSL/Async/ParallelPool.h>
   void func1()
   {
        //long running function that has side effects (i.e. writing to file)
   }

   void func2(int arg)
   {
        //long running function that takes an arg
   }

   double func3(int arg)
   {
        //funciton with a return value
        return 1.0/(1.0 + arg*arg); 
   }

   int main(int argc, char ** argv)
   {
        int Nworkers = 3; 
        
        JSL::Async::Pool pool(Nworkers);

        ///////////////////
        // Simple dispatch
        ///////////////////
        pool.AsyncTask(func1);
        for (int i = 0; i < 10; ++i) //pool maintains a queue of tasks, so we don't need to worry about 'overloading'
        {
            pool.AsyncTask([i](){func2(i)}); // have to provide args to tasks by wrapping them in a lambda
        }
        pool.Synchronise(); //wait until all 11 tasks have been completed, then continue

        // the pool is now dormant, but can be re-used
        
        ///////////////////////
        // Return values
        /////////////////////// 
        std::future<double> y1 = pool.AsyncReturn<double>([](){func3(0);});
        std::future<double> y2 = pool.AsyncReturn<double>([](){func3(1);});

        // y1 and y2 are now promises that a future value will be assigned...
         
        pool.Synchronise(); //ensure all promises have been met
        LOG(INFO) << "y1 = " << y1.get() << " y2 = " << y2.get();

        //////////////////////
        // Loop tasks
        //////////////////////

        pool.LoopTask(100,func3); // the same as the for loop above, but with less overhead 
        //loop task is blocking, waits until the loop is complete

        std::vector<double>  y = pool.LoopReturn(100,func3,/*optional distribution policy*/);
        // the value of y[i] = func2(i)
   }
