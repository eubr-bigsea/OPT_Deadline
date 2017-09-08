-- DAG definition: it is encoded as an array of stages.
Stages = {{ name="S1", tasks="2", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/2apps/app_files/testP8/S1.txt")}}, pre={}, post={"S5","S6","S4"}},{ name="S0", tasks="2", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/2apps/app_files/testP8/S0.txt")}}, pre={}, post={"S5","S6","S4"}},{ name="S3", tasks="2", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/2apps/app_files/testP8/S3.txt")}}, pre={}, post={"S5","S6","S4"}},{ name="S2", tasks="2", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/2apps/app_files/testP8/S2.txt")}}, pre={}, post={"S5","S6","S4"}},{ name="S5", tasks="3000", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/2apps/app_files/testP8/S5.txt")}}, pre={"S1","S0","S3","S2"}, post={"S7"}},{ name="S4", tasks="2", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/2apps/app_files/testP8/S4.txt")}}, pre={"S1","S0","S3","S2"}, post={"S8"}},{ name="S7", tasks="200", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/2apps/app_files/testP8/S7.txt")}}, pre={"S5","S6"}, post={"S8"}},{ name="S6", tasks="1000", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/2apps/app_files/testP8/S6.txt")}}, pre={"S1","S0","S3","S2"}, post={"S7"}},{ name="S9", tasks="200", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/2apps/app_files/testP8/S9.txt")}}, pre={"S8"}, post={}},{ name="S8", tasks="200", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/2apps/app_files/testP8/S8.txt")}}, pre={"S4","S7"}, post={"S9"}}};

-- Number of computation nodes in the system
Nodes = @@nodes@@;

-- Number of users accessing the system
Users = 1;

-- Distribution of the think time for the users
UThinkTimeDistr = {type = "exp", params = {rate = 0.001}};

-- Total number of jobs to simulate
maxJobs = 1000;

-- Coefficient for the Confidence Intervals
confIntCoeff = 1.96;
