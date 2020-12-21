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
        docker { 
            image 'lsstts/mtm1m3_sim:latest'
            args '--entrypoint="" -u root'
        }
    }

    environment {
        SALUSER_HOME = "/home/saluser"
    }

    stages {
        stage('Cloning source') {
            steps {
                checkout scm
            }
        }

        stage('Test') {
            steps {
                sh """
                    source $SALUSER_HOME/.setup_salobj.sh

                    export PATH=/opt/lsst/software/stack/python/miniconda3-4.7.12/envs/lsst-scipipe-448abc6/bin:$PATH

                    make junit
                """
            }
        }

        junit 'tests/*.xml
    }
}
