#!/usr/bin/env groovy

properties(
    [
    buildDiscarder
        (logRotator (
            artifactDaysToKeepStr: '',
            artifactNumToKeepStr: '',
            daysToKeepStr: '14',
            numToKeepStr: ''
        ) ),
    disableConcurrentBuilds(),
    parameters
        ( [
            booleanParam(defaultValue: false, description: 'Adds --no-cache to Docker build command', name: 'noCache'),
            booleanParam(defaultValue: false, description: 'Calls make clean before building the code', name: 'clean')
        ] )
    ]
)

node {


    def SALUSER_HOME = "/home/saluser"
    def BRANCH = (env.CHANGE_BRANCH != null) ? env.CHANGE_BRANCH : env.BRANCH_NAME

    stage('Cloning sources')
    {
        dir("ts_cRIOcpp") {
            checkout scm
        }
    }

    stage('Building dev container')
    {
        M1M3sim = docker.build("lsstts/criocpp:" + env.BRANCH_NAME.replace("/", "_"), (params.noCache ? "--no-cache " : " ") + "ts_cRIOcpp")
    }

    stage("Running tests")
    {
        withEnv(["SALUSER_HOME=" + SALUSER_HOME]) {
             M1M3sim.inside("--entrypoint=''") {
                 if (params.clean) {
                 sh """
                    cd $WORKSPACE/ts_cRIOcpp
                    make clean
                 """
                 }
                 sh """
                    source $SALUSER_HOME/.setup_salobj.sh
    
                    cd $WORKSPACE/ts_cRIOcpp

                    export PATH=\$CONDA_PREFIX/bin:$PATH
                    export LIBS="-L\$CONDA_PREFIX/lib" 
                    export CPP_FLAGS="-I\$CONDA_PREFIX/include"
                    export PKG_CONFIG_PATH="\$CONDA_PREFIX/lib/pkgconfig"
                    make
                    make junit
                 """
             }
        }

        junit 'ts_cRIOcpp/tests/*.xml'
    }

    stage('Build documentation')
    {
         M1M3sim.inside("--entrypoint=''") {
             sh """
                source $SALUSER_HOME/.setup_salobj.sh
                cd $WORKSPACE/ts_cRIOcpp
                make doc
             """
         }
    }

    if (BRANCH == "master" || BRANCH == "develop")
    {
        stage('Publish documentation')
        {
            withCredentials([usernamePassword(credentialsId: 'lsst-io', usernameVariable: 'LTD_USERNAME', passwordVariable: 'LTD_PASSWORD')]) {
                M1M3sim.inside("--entrypoint=''") {
                    sh """
                        source $SALUSER_HOME/.setup_salobj.sh
                        ltd upload --product ts-criocpp --git-ref """ + BRANCH + """ --dir $WORKSPACE/ts_cRIOcpp/doc/html
                    """
                }
            }
        }
    }
}
