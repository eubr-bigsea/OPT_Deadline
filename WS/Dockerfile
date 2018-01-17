FROM ubuntu:16.04

ENV OPT_DEADLINE_WS_HOME /opt/OPT_DEADLINE_WS

ENV DAGSIM_REPOSITORY https://github.com/eubr-bigsea/dagSim
ENV DAGSIM_HOME /opt/dagSim

ENV OPT_DEADLINE_REPOSITORY https://github.com/eubr-bigsea/OPT_Deadline
ENV OPT_DEADLINE_HOME /opt/OPT_DEADLINE

ENV OPT_COMMON_REPOSITORY https://github.com/eubr-bigsea/OPT_Common
ENV OPT_COMMON_HOME /opt/OPT_Common

ENV OPT_IC_REPOSITORY https://github.com/eubr-bigsea/OPT_IC
ENV OPT_IC_HOME /opt/OPT_IC


RUN apt-get update && apt-get install -y nano vim git wget make gcc liblua5.3-dev python3.5

RUN mkdir /tarball

# Download and install DagSim
RUN cd /tarball && \
    git clone ${DAGSIM_REPOSITORY} && \
    cd dagSim && \
    git checkout cd2c66b268bb2bf82c04c08c1e56a462c95ae03f && \
    make clean && \
    make && \
    chmod ugo+x dagsim.sh && \
    mkdir -p ${DAGSIM_HOME} && \
    cp -r ./* ${DAGSIM_HOME}/ && \
    chown -R ${WSI_USER}:${WSI_USER} ${DAGSIM_HOME}

RUN cd /tarball && \
    git clone ${OPT_COMMON_REPOSITORY} && \
    cd OPT_Common && \
    git checkout 7ae6470d1eb2ca46909744ecfd01601b9e1779b4 && \
    mkdir -p ${OPT_COMMON_HOME} && \
    cp -r ./* ${OPT_COMMON_HOME}/

# Download and install OPT_IC
RUN cd /tarball && \
    git clone ${OPT_IC_REPOSITORY} && \
    cd OPT_IC && \
    git checkout 4c12bd23f401d05a0168dc4bfa3c2fc667020fd5 && \
    make CXXINCLUDES=-I${OPT_COMMON_HOME}/include && \
    cp -r /tarball/OPT_IC /opt && \
    chown -R ${WSI_USER}:${WSI_USER} ${OPT_IC_HOME} && \
    chmod ugo+x ${OPT_IC_HOME}/script.sh && \
    echo "${OPT_IC_HOME}/data/" > ${OPT_IC_HOME}/config.txt && \
    echo "${DAGSIM_HOME}/" >> ${OPT_IC_HOME}/config.txt && \
    echo "${OPT_IC_HOME}/lua/" >> ${OPT_IC_HOME}/config.txt

RUN cd /tarball && \
    git clone ${OPT_DEADLINE_REPOSITORY} && \
    cd OPT_Deadline && \
    git checkout b6189df61c23e4c4cbe5aaa36f7d8423f9f3d067 && \
    make OPT_COMMON_INCLUDE=-I${OPT_COMMON_HOME}/include && \
    mkdir ${OPT_DEADLINE_HOME} && \
    cp -r /tarball/OPT_Deadline/* ${OPT_DEADLINE_HOME}

RUN rm -rf /tarball

RUN mkdir ${OPT_DEADLINE_WS_HOME} && \
    cd ${OPT_DEADLINE_WS_HOME}


RUN apt-get install -y python3-pip
RUN pip3 install --upgrade pip

COPY requirements.txt ./
RUN pip3 install --no-cache-dir -r requirements.txt


RUN mkdir -p /opt/OPT_DEADLINE_WS
WORKDIR /opt/OPT_DEADLINE_WS

COPY . /opt/OPT_DEADLINE_WS/

EXPOSE 8080

CMD [ "python3.5", "run_server.py" ]
