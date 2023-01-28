/*
 * Replace the following string of 0s with your student number
 * 210167180
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "joblog.h"

/* 
 * DO NOT EDIT the new_log_name function. It is a private helper 
 * function provided for you to create a log name from a process 
 * descriptor for use when reading, writing and deleting a log file.
 * 
 * You must work out what the function does in order to use it properly
 * and to clean up after use.
 */
static char* new_log_name(proc_t* proc) {
    static char* joblog_name_fmt = "%s/%.31s%07d.txt";
                                // string format for the name of a log file
                                // declared static to have only one instance

    if (!proc)
        return NULL;

    char* log_name;
            
    asprintf(&log_name, joblog_name_fmt, JOBLOG_PATH, proc->type_label,
        proc->id);//the same as snprintf except it returns a pointer to the memory
					// allocated 

    return log_name;
}

/* 
 * DO NOT EDIT the joblog_init function that sets up the log directory 
 * if it does not already exist.
 */
int joblog_init(proc_t* proc) {
    if (!proc) {
        errno = EINVAL;
        return -1;
    }
        
    int r = 0;
    if (proc->is_init) {
        struct stat sb;
    
        if (stat(JOBLOG_PATH, &sb) != 0) {
            errno = 0;
            r = mkdir(JOBLOG_PATH, 0777);
        }  else if (!S_ISDIR(sb.st_mode)) {
            unlink(JOBLOG_PATH);
            errno = 0;
            r = mkdir(JOBLOG_PATH, 0777);
        }
    }

    joblog_delete(proc);    // in case log exists for proc
    
    return r;
}

/* 
 * TODO: you must implement this function.
 * Hints:
 * - you have to go to the beginning of the line represented
 *      by entry_num to read the required entry
 * - see job.h for a function to create a job from its string representation
 */
job_t* joblog_read(proc_t* proc, int entry_num, job_t* job) {
	
	if (proc == NULL || entry_num < 0){
		return NULL;
	}
	
	char* fileName = new_log_name(proc);
	
	if (fileName == NULL){
		return NULL;
	}
	
	int init_errno = errno;
	FILE* f = fopen(fileName,"r");
	
	if (f==NULL){
		errno = init_errno;
		return NULL;
	}
	
	char* str = malloc(JOB_STR_SIZE);
	
	int r = fseek(f,entry_num*JOB_STR_SIZE,SEEK_SET);
	
	if (r == -1){
		free(str);
		errno = init_errno;
		return NULL;
	}
	
	fgets(str,JOB_STR_SIZE,f);	
	job = str_to_job(str,job);
	free(str);
	fclose(f);
	
	if (job == NULL){
		return NULL;
	}

    return job;
}

/* 
 * TODO: you must implement this function.
 * Hints:
 * - remember new entries are appended to a log file
 * - see the hint for joblog_read
 */
void joblog_write(proc_t* proc, job_t* job) {
	
	if (proc == NULL || job == NULL){
		return;
	}
	
	char* fileName = new_log_name(proc);
	if (fileName == NULL){
		return;
	}

	FILE* f = fopen(fileName,"a");
	
	if (f == NULL){
		return;
	}
	
	char* str = NULL;
	str = job_to_str(job,str);
	
	int init_errno = errno;
	if (fprintf(f,"%s\n",str) < 0){
		fclose(f);
		errno = init_errno;
		free(str);
		free(fileName);
		return;
	}
	
	fclose(f);
	free(str);
	free(fileName);
	
    return;
}

/* 
 * TODO: you must implement this function.
 */
void joblog_delete(proc_t* proc) {
	char* log = new_log_name(proc);
	remove(log);
    return;
}