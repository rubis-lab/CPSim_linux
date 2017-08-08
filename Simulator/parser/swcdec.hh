Task_info *t0 = new Task_info(0, 0, 50000, 20000, 4000, 0, 1, 1, 1, 50000, -1, "CC1", s0);
whole_tasks.push_back(t0);
Task_info *t1 = new Task_info(1, 0, 200000, 30000, 6000, 0, 1, 1, 1, 200000, -1, "LK1", s1);
whole_tasks.push_back(t1);
Task_info *t2 = new Task_info(2, 0, 100000, 50000, 10000, 0, 1, 1, 1, 100000, -1, "CC2", s0);
whole_tasks.push_back(t2);
Task_info *t3 = new Task_info(3, 0, 200000, 50000, 10000, 0, 1, 1, 1, 200000, -1, "LK2", s1);
whole_tasks.push_back(t3);
Task_info *t4 = new Task_info(4, 0, 40000, 10000, 2000, 0, 1, 1, 1, 40000, -1, "other1", s2);
whole_tasks.push_back(t4);
Task_info *t5 = new Task_info(5, 0, 80000, 15000, 3000, 0, 1, 1, 1, 80000, -1, "other2", s2);
whole_tasks.push_back(t5);
Task_info *t6 = new Task_info(6, 0, 100000, 20000, 4000, 0, 1, 1, 1, 100000, -1, "other3", s2);
whole_tasks.push_back(t6);
Task_info *t7 = new Task_info(7, 0, 250000, 120000, 24000, 0, 1, 1, 1, 250000, -1, "LK3", s1);
whole_tasks.push_back(t7);

//data dependency
t0->successors.push_back(t2);
t2->predecessors.push_back(t0);
t1->successors.push_back(t7);
t7->predecessors.push_back(t1);
t3->successors.push_back(t7);
t7->predecessors.push_back(t3);
