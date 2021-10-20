FROM lsstts/develop-env:develop

USER root
RUN yum -y install catch-devel boost169-devel make readline-devel

USER saluser

WORKDIR /home/saluser

USER root
RUN chmod a+rwX -R /home/saluser/
USER saluser

RUN source .setup.sh \
    && conda install -y readline yaml-cpp boost-cpp

SHELL ["/bin/bash", "-lc"]
