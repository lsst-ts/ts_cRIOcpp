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
            image 'centos/devtoolset-7-toolchain-centos7'
            args '-u root'
        }
    }

    stages {
        stage('Install dependencies') {
            steps {
                sh """
                    yum install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
                    yum install -y make boost-devel catch-devel
                """
            }
        }

        stage('Cloning source') {
            steps {
                checkout scm
            }
        }

        stage('Test') {
            steps {
                sh """
                    make
                    make junit
                """

                junit 'tests/*.xml'
            }
        }

    }
}
