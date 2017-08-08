//create resources
Resource *r0 = new Resource(0, 1, 100, 100, "ecu1");
resources.push_back(r0);
Scheduler *s0 = new Scheduler(0, 1, r0);
Resource *r1 = new Resource(1, 1, 100, 180, "ecu2");
resources.push_back(r1);
Scheduler *s1 = new Scheduler(1, 1, r1);

// create tasks
Task_info *t0 = new Task_info(0, 0, 250000, 50000, 10000, 0, 1, 1, 1, 250000, -1, "LKAS", s0);
whole_tasks.push_back(t0);
Task_info *t1 = new Task_info(1, 0, 30000, 12000, 2400, 0, 1, 0, 0, 30000, -1, "dummy1", s0);
whole_tasks.push_back(t1);
Task_info *t2 = new Task_info(2, 0, 40000, 16000, 3200, 0, 1, 0, 0, 40000, -1, "dummy2", s0);
whole_tasks.push_back(t2);
Task_info *t3 = new Task_info(3, 0, 30000, 5000, 1000, 0, 1, 1, 0, 30000, -1, "CC1", s1);
whole_tasks.push_back(t3);
Task_info *t4 = new Task_info(4, 0, 30000, 5000, 1000, 0, 1, 0, 1, 30000, -1, "CC2", s1);
whole_tasks.push_back(t4);

// set data dependency
t3->successors.push_back(t4);
t4->predecessors.push_back(t3);
