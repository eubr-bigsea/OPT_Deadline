-- DAG definition: it is encoded as an array of stages.
Stages = {{ name="S1", tasks="2", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/4apps/app_files/testD/S1.txt")}}, pre={}, post={"S3","S4"}},{ name="S0", tasks="2", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/4apps/app_files/testD/S0.txt")}}, pre={}, post={"S3","S4"}},{ name="S3", tasks="500", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/4apps/app_files/testD/S3.txt")}}, pre={"S1","S0","S2"}, post={"S5"}},{ name="S2", tasks="2", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/4apps/app_files/testD/S2.txt")}}, pre={}, post={"S3","S4"}},{ name="S5", tasks="200", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/4apps/app_files/testD/S5.txt")}}, pre={"S3","S4"}, post={"S6"}},{ name="S4", tasks="500", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/4apps/app_files/testD/S4.txt")}}, pre={"S1","S0","S2"}, post={"S5"}},{ name="S6", tasks="200", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/4apps/app_files/testD/S6.txt")}}, pre={"S5"}, post={}}};

-- Number of computation nodes in the system
Nodes = @@nodes@@;

-- Number of users accessing the system
Users = 1;

-- Distribution of the think time for the users
UThinkTimeDistr = {type = "exp", params = {rate = 0.001}};

-- Total number of jobs to simulate
maxJobs = 1000;

-- Coefficient for the Confidence Intervals
confIntCoeff =  1.96;
