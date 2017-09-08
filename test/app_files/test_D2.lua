-- DAG definition: it is encoded as an array of stages.
Stages = {{ name="S1", tasks="2", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/app_files/testD2/S1.txt")}}, pre={}, post={"S2"}},{ name="S0", tasks="2", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/app_files/testD2/S0.txt")}}, pre={}, post={"S2"}},{ name="S3", tasks="200", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/app_files/testD2/S3.txt")}}, pre={"S2"}, post={}},{ name="S2", tasks="500", distr={type="empirical", params={samples=solver.fileToArray("/home/biagio/repositories/OPT_Deadline/test/app_files/testD2/S2.txt")}}, pre={"S1","S0"}, post={"S3"}}};

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
