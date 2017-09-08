-- DAG definition: it is encoded as an array of stages.
Stages = {{ name="S14", tasks="7", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/4apps/app_files/testP82/S14.txt")}}, pre={"S9"}, post={}},{ name="S1", tasks="2", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/4apps/app_files/testP82/S1.txt")}}, pre={"S0"}, post={"S3"}},{ name="S0", tasks="2", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/4apps/app_files/testP82/S0.txt")}}, pre={}, post={"S1","S2"}},{ name="S3", tasks="200", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/4apps/app_files/testP82/S3.txt")}}, pre={"S1","S2"}, post={"S4"}},{ name="S2", tasks="3958", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/4apps/app_files/testP82/S2.txt")}}, pre={"S0"}, post={"S3"}},{ name="S4", tasks="200", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/4apps/app_files/testP82/S4.txt")}}, pre={"S3"}, post={"S8"}},{ name="S9", tasks="1", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/4apps/app_files/testP82/S9.txt")}}, pre={"S8"}, post={"S14"}},{ name="S8", tasks="200", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/4apps/app_files/testP82/S8.txt")}}, pre={"S4"}, post={"S9"}}};

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
