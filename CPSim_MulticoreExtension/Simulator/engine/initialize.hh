// create resources
Resource *r1 = new Resource(0, 1, 100, 180, "ecu1");
r1->scheduler_link = new Scheduler_Universal(2, 1);
resources.push_back(r1);
Resource *r2 = new Resource(1, 1, 100, 180, "ecu2");
r2->scheduler_link = new Scheduler_Universal(2, 1);
resources.push_back(r2);

// create task metadata
Task_info *t1 = new Task_info(0, 0, 0, 50000, 5000, 10000, 25, 0, 1, 1, 0, 50000, -1, "CC1");
r1->scheduler_link->add_task(t1);
whole_tasks.push_back(t1);
Task_info *t2 = new Task_info(1, 0, 0, 100000, 10000, 30000, 25, 0, 1, 0, 1, 100000, -1, "CC2");
r1->scheduler_link->add_task(t2);
whole_tasks.push_back(t2);
Task_info *t3 = new Task_info(2, 0, 1, 100000, 10000, 20000, 25, 0, 1, 1, 0, 100000, -1, "LK1");
r2->scheduler_link->add_task(t3);
whole_tasks.push_back(t3);
Task_info *t4 = new Task_info(3, 0, 1, 200000, 20000, 30000, 25, 0, 1, 0, 0, 200000, -1, "LK2");
r2->scheduler_link->add_task(t4);
whole_tasks.push_back(t4);
Task_info *t5 = new Task_info(4, 0, 1, 400000, 50000, 200000, 25, 0, 1, 0, 1, 400000, -1, "LK3");
r2->scheduler_link->add_task(t5);
whole_tasks.push_back(t5);

// create task functions
Task *t1_function = new Task1();
whole_task_functions.push_back(t1_function);
Task *t2_function = new Task2();
whole_task_functions.push_back(t2_function);
Task *t3_function = new Task3();
whole_task_functions.push_back(t3_function);
Task *t4_function = new Task4();
whole_task_functions.push_back(t4_function);
Task *t5_function = new Task5();
whole_task_functions.push_back(t5_function);

// set data dependency for CC
t1->successors.push_back(t2);
t2->predecessors.push_back(t1);

// set data dependency for LK
t3->successors.push_back(t4);
t4->predecessors.push_back(t3);
t4->successors.push_back(t5);
t5->predecessors.push_back(t4);
