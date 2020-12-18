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
            booleanParam(defaultValue: false, description: 'Calls make clean before building the code', name: 'clean')
        ] )
    ]
)

pipeline {
    agent {
        docker { image 'lsstts/mtm1m3_sim:latest' }
    }

    def SALUSER_HOME = "/home/saluser"
    def BRANCH = (env.CHANGE_BRANCH != null) ? env.CHANGE_BRANCH : env.BRANCH_NAME

    stages {
        stage('Cloning source') {
            dir('ts_cRIOcpp') {
                git branch: BRANCH, url: 'https://github.com/lsst-ts/ts_cRIOcpp'
            }
        }

        stage('Test') {
            if (params.clean) {
                sh 'make clean'
            }

            dir('ts_cRIOcpp') {
                sh 'source $SALUSER_HOME/.setup_salobj.sh && make run_tests'
            }
        }
    }
}
