#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <limits.h>

// TODO: Add more fields to this struct
struct job {
    int id;
    int arrival;
    int length;
    int initial_length;
    float start_time;
    float completion_time;
    int has_been_run;
    struct job *next;
};

// Current stauts: logic for the three polocies exists but neeeds more testing

void policy_SJF(struct job *head);
void policy_FIFO(struct job *head);
void policy_RR(struct job *head, int slice_duration);

void analyze_FIFO(struct job *head);
void analyze_SJF(struct job *head);
void analyze_RR(struct job *head);

struct job* read_workload_file(char* filename);
struct job* grab_shortest(struct job* head, int time);
struct job* grab_next_RR(struct job* head, struct job* current, int time, int time_slice);

int main(int argc, char **argv) {

 if (argc < 4) {
    fprintf(stderr, "missing variables\n");
    fprintf(stderr, "usage: %s analysis-flag policy workload-file slice-duration\n", argv[0]);
		exit(EXIT_FAILURE);
  }

  int analysis = atoi(argv[1]);
  char *policy = argv[2],
       *workload = argv[3];
  int slice_duration = atoi(argv[4]);

  // the start of a linked-list of jobs, i.e., the job list 
  struct job* head = read_workload_file(workload);

  // run the FIFO
  if (strcmp(policy, "FIFO") == 0 ) {
    policy_FIFO(head);
    if (analysis) {
      printf("Begin analyzing FIFO:\n");
      analyze_FIFO(head);
      printf("End analyzing FIFO.\n");
    }

    exit(EXIT_SUCCESS);
  }

  // run the SJF
  if (strcmp(policy, "SJF") == 0 ) {
    policy_SJF(head);
    if (analysis) {
      printf("Begin analyzing SJF:\n");
      analyze_SJF(head);
      printf("End analyzing SJF.\n");
    }

    exit(EXIT_SUCCESS);
  }

  // run the RR
  if (strcmp(policy, "RR") == 0 ) {
    policy_RR(head, slice_duration);
    if (analysis) {
      printf("Begin analyzing RR:\n");
      analyze_RR(head);
      printf("End analyzing RR.\n");
    }

    exit(EXIT_SUCCESS);
  }
}

/*Function to read in the workload file and create job list*/
struct job* read_workload_file(char* filename){
  int id = 0;
  FILE *fp;
  size_t len = 0;
  ssize_t read;
  char *line = NULL,
       *arrival = NULL, 
       *length = NULL;


  struct job* head = NULL;
  struct job* current = NULL;

  if( (fp = fopen(filename, "r")) == NULL)
    exit(EXIT_FAILURE);

  while ((read = getline(&line, &len, fp)) > 1) {
    arrival = strtok(line, ",\n");
    length = strtok(NULL, ",\n");

       
    // Make sure neither arrival nor length are null. 
    assert(arrival != NULL && length != NULL);
        
    if (head == NULL){
      
      head = (struct job*)malloc(sizeof(struct job));
      head->id = id;
      head->arrival = atoi(arrival);
      head->length = atoi(length);
      head->start_time = 0;
      head->completion_time = 0;
      head->has_been_run = 0;
      head->next = NULL;
      current = head;

      id++;

    }else{

      struct job *new_node = (struct job*) malloc(sizeof(struct job));
      new_node->id = id;
      new_node->arrival = atoi(arrival);
      new_node->length = atoi(length);
      head->start_time = 0;
      head->completion_time = 0;
      head->has_been_run = 0;
      new_node->next = NULL;
      current->next = new_node;
      current = new_node;

      id++;
    }
  }

  fclose(fp);

  // Make sure we read in at least one job
  assert(id > 0);

  return head;
}

// the FIFO policy
void policy_FIFO(struct job *head) {

  printf("Execution trace with FIFO:\n");
  struct job *current = head;
  int time = 0;
  while (current != NULL) {
      //print the features of it
      if (current->arrival > time){
        time = current->arrival;
      }
      current->start_time = (float)time;
      printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n",time, current->id, current->arrival, current->length);
      time = time + current->length;
      current->completion_time = (float)time;
      current = current->next;
  }
  printf("End of execution with FIFO.\n");
}

// the SJF policy
void policy_SJF(struct job *head){
  printf("Execution trace with SJF:\n");;
  int time = 0;
  struct job* current = grab_shortest(head, time);
  while (current != NULL){
    if (current->arrival > time){ time = current->arrival;}

    current->start_time = (float)time;
    printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n",time, current->id, current->arrival, current->length);
    time = time + current->length;
    current->completion_time = (float)time;
    current->length = 0;
    current = grab_shortest(head, time);
  }
  printf("End of execution with SJF.\n");
}


// grabs the shortest node which arrived at or before the time. If none exsits, it grabs the next one which arrives w/ the shortest time.
struct job* grab_shortest(struct job* head, int time){

  struct job* min = NULL;
  struct job *current = head;


  // grab the shortest one which arrived before the time
  while(current != NULL){

    if (min == NULL && current->arrival <= time && current->length > 0){
      min = current;
    }

    if (min != NULL){
      if (current->length < min->length && current->length > 0 && current->arrival <= time){
        min = current;
      }
    }
    current = current->next;
  }

  if (min != NULL){return min;}


  // reset current
  current = head;
  while(current != NULL)
  {
    if (min == NULL && current->length > 0){
      min = current;
    }
    if (min != NULL){
      if (current->arrival < min->arrival && current->length > 0){
        min = current;
      }
      if (current->arrival == min->arrival && current->length > 0){
        if (current->length < min->length){
          min = current;
        }
      }
    }
    current = current->next;
  }
  return min;
}


void policy_RR(struct job *head, int slice_duration){
  printf("Execution trace with RR:\n");;
  int time = 0;
  struct job* current = head;
  while (current != NULL){

    if (current->arrival > time){ time = current->arrival;}

    int time_run = slice_duration;
    if (current->length < slice_duration){
      time_run = current->length;
    }

    if (current->has_been_run == 0){
      current->start_time = (float)time;
      current->initial_length = current->length;
      current->has_been_run = 1;
    }

    printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n",time, current->id, current->arrival, time_run);
    time = time + time_run;
    current->length = current->length - time_run;

    if (current->length == 0){
      current->completion_time = (float)time;
    }

    current = grab_next_RR(head, current, time, slice_duration);
  }
  printf("End of execution with RR.\n");
}

// if the next is 
struct job* grab_next_RR(struct job* head, struct job* current, int time, int time_slice){


  struct job* choice = current->next;
  //first loop from current to the end
  while(choice != NULL){
    if (choice->length > 0 && choice->arrival <= time){
      return choice;
    }
    choice = choice->next;
  }
  choice = head;
  while(choice != current){
    if (choice->length > 0){
      return choice;
    }
    choice = choice->next;
  }

  //either we are all done or we need to wait

  choice = head;
  while (choice != NULL){
    if (choice->length > 0){
      return choice;
    }
    choice = choice->next;
  }
  return NULL;
}

void analyze_FIFO(struct job *head) {
  struct job* current = head; 

  int total_elements = 0;
  float response_time_total = 0;
  float turnaounrd_time_total = 0;
  float wait_time_total = 0;

  while (current != NULL){
    int id = current->id;
    float response_time = current->start_time - current->arrival;
    float turnaounrd_time = current->completion_time - current->arrival;
    float wait_time = response_time;

    total_elements++;
    response_time_total = response_time_total + response_time;
    turnaounrd_time_total = turnaounrd_time_total + turnaounrd_time;
    wait_time_total = wait_time_total + wait_time;
    printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", id, (int)response_time, (int)turnaounrd_time, (int)wait_time);
    current = current->next;
  }
  printf("Average -- Response: %0.2f  Turnaround %0.2f  Wait %0.2f\n",response_time_total/total_elements, turnaounrd_time_total/total_elements, wait_time_total/total_elements);
}

void analyze_SJF(struct job *head) {
  struct job* current = head; 

  int total_elements = 0;
  float response_time_total = 0;
  float turnaounrd_time_total = 0;
  float wait_time_total = 0;

  while (current != NULL){
    int id = current->id;
    float response_time = current->start_time - current->arrival;
    float turnaounrd_time = current->completion_time - current->arrival;
    float wait_time = response_time;

    total_elements++;
    response_time_total = response_time_total + response_time;
    turnaounrd_time_total = turnaounrd_time_total + turnaounrd_time;
    wait_time_total = wait_time_total + wait_time;
    printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", id, (int)response_time, (int)turnaounrd_time, (int)wait_time);
    current = current->next;
  }
  printf("Average -- Response: %0.2f  Turnaround %0.2f  Wait %0.2f\n",response_time_total/total_elements, turnaounrd_time_total/total_elements, wait_time_total/total_elements);
}


void analyze_RR(struct job *head) {
  struct job* current = head; 

  int total_elements = 0;
  float response_time_total = 0;
  float turnaounrd_time_total = 0;
  float wait_time_total = 0;

  while (current != NULL){
    int id = current->id;
    float response_time = current->start_time - current->arrival;
    float turnaounrd_time = current->completion_time - current->arrival;
    float wait_time = turnaounrd_time - current->initial_length;

    total_elements++;
    response_time_total = response_time_total + response_time;
    turnaounrd_time_total = turnaounrd_time_total + turnaounrd_time;
    wait_time_total = wait_time_total + wait_time;
    printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", id, (int)response_time, (int)turnaounrd_time, (int)wait_time);
    current = current->next;
  }
  printf("Average -- Response: %0.2f  Turnaround %0.2f  Wait %0.2f\n",response_time_total/total_elements, turnaounrd_time_total/total_elements, wait_time_total/total_elements);
}
