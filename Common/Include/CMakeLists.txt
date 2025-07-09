CMAKE_MINIMUM_REQUIRED (VERSION 2.8.10)

EXECUTE_PROCESS(COMMAND gdbus-codegen --generate-c-code testservice --c-namespace TestService ../../Common/Include/org.example.ITestService.xml)  