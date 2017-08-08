Resource *r0 = new Resource(0, 1, 100, 100, "ECU0");
resources.push_back(r0);
Scheduler *s0 = new Scheduler(RM, 1, r0);
Resource *r1 = new Resource(1, 1, 100, 100, "ECU1");
resources.push_back(r1);
Scheduler *s1 = new Scheduler(RM, 1, r1);
Resource *r2 = new Resource(2, 1, 100, 100, "ECU2");
resources.push_back(r2);
Scheduler *s2 = new Scheduler(RM, 1, r2);
