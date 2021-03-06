
#include <iostream>
#include <stdlib.h>
#include <sys/resource.h>

#include "sim_services/Executive/include/Executive.hh"
#include "sim_services/Executive/include/Exec_exception.hh"
#include "sim_services/Executive/include/exec_proto.h"

/**
@details
-# Set the mode to Initialization
   Requirement [@ref r_exec_mode_0].
-# Start the cpu usage meter
-# Call the default_data jobs
-# Call the initialization jobs.
-# Record the cpu usage during initialization
-# The scheduler initializes simulation timers.
   Requirement [@ref r_exec_time_1].
-# If an exception is caught, print as much error information available based
   on execption type caught and exit.
-# If no execption is caught return 0
*/
int Trick::Executive::init() {

    double cpu_time ;

    try {

        mode = Initialization ;

        /* Start the cpu usage meter */
        struct rusage cpu_usage_buf ;
        getrusage(RUSAGE_SELF, &cpu_usage_buf);
        cpu_start =   ((double) cpu_usage_buf.ru_utime.tv_sec) + ((double) cpu_usage_buf.ru_utime.tv_usec / 1000000.0);

        call_default_data() ;

        call_input_processor() ;

        // If we are starting from a checkpoint, restart_called will be true.  Skip init routines in this case.
        if ( ! restart_called ) {
            call_initialization() ;
        }

        /* Set the initial values for the scheduler times. */
        next_frame_check_tics = software_frame_tics + time_tics ;
        job_call_time_tics = next_frame_check_tics ;
        sim_start = get_sim_time();

        /* Record the cpu usage for initialization */
        getrusage(RUSAGE_SELF, &cpu_usage_buf);
        cpu_time = ((double) cpu_usage_buf.ru_utime.tv_sec) + ((double) cpu_usage_buf.ru_utime.tv_usec / 1000000.0);
        cpu_init = cpu_time - cpu_start;

        initialization_complete = true ;

    /* Print as much error information avaiable for all exception and exit. */
    } catch (Trick::Exec_exception & ex ) {
        /* Set the exit return code, file name, and error message. Return -1 so we go to shutdown */
        except_return = ex.ret_code ;
        except_file = ex.file ;
        except_message = ex.message ;
        return(-1) ;
    } catch (const std::exception &ex) {
        if ( curr_job != NULL ) {
            except_file = curr_job->name ;
        } else {
            except_file = "somewhere in Executive::init" ;
        }
        fprintf(stderr, "\nExecutive::loop terminated with std::exception\n  ROUTINE: %s\n  DIAGNOSTIC: %s\n",
         except_file.c_str(), ex.what()) ;
        exit(-1) ;
    } catch (...) {
        if ( curr_job != NULL ) {
            except_file = curr_job->name ;
        } else {
            except_file = "somewhere in Executive::init" ;
        }
        except_message = "unknown error" ;
        fprintf(stderr, "\nExecutive::loop terminated with unknown exception\n  ROUTINE: %s\n  DIAGNOSTIC: %s\n",
         except_file.c_str() , except_message.c_str()) ;
        exit(-1) ;
    }

    /* return 0 if there are no errors. */
    return(0) ;
}

