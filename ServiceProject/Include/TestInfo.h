// Common/Include/TestInfo.h
#pragma once
#include <string>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <vector>
#include <fcntl.h>    
#include <sys/mman.h> 
#include <sys/mount.h>
#include <pthread.h>
#include <openssl/md5.h>
#include <gio/gio.h>
#include <glib.h>
#include "testservice.h"
#include <mutex>
#include <iostream>



#define ORG_EXAMPLE_ITESTSERVICE_BUS        G_BUS_TYPE_SESSION
#define ORG_EXAMPLE_ITESTSERVICE_NAME    "org.example.ITestService"
#define ORG_EXAMPLE_ITESTSERVICE_OBJECT_PATH   "/org/example/ITestService"

struct TestInfo {
    bool bool_param;
    int int_param;
    double double_param;
    std::string string_param;
};

GVariant* to_variant(const TestInfo& info);
void from_variant(GVariant* variant, TestInfo& info);
std::string calculate_md5(const void* data, size_t len);


gboolean handleSetTestBool(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gboolean param, gpointer user_data);

gboolean handleSetTestInt(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gint32 param, gpointer user_data);

gboolean handleSetTestDouble(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gdouble param, gpointer user_data);

gboolean handleSetTestString(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, const gchar *param, gpointer user_data);

gboolean handleSetTestInfo(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv,
                                  GVariant *param, gpointer user_data);

gboolean handleGetTestBool(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gpointer user_data);

gboolean handleGetTestInt(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gpointer user_data);

gboolean handleGetTestDouble(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gpointer user_data);

gboolean handleGetTestString(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gpointer user_data);

gboolean handleGetTestInfo(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gpointer user_data) ;

gboolean handleSendFileMetadata(TestServiceOrgExampleITestService *skeleton,
                                       GDBusMethodInvocation *inv,
                                       const gchar *filename,
                                       guint32 filesize,
                                       const gchar *md5,
                                       gpointer user_data);


gboolean handleSendFileNotification(TestServiceOrgExampleITestService *skeleton,
                                           GDBusMethodInvocation *inv,
                                           const gchar *shm_name,
                                           guint32 offset,
                                           guint32 size,
                                           gboolean is_last_chunk,
                                           gpointer user_data);


void GBusAcquired_Callback(GDBusConnection *connection,
                         const gchar *name,
                         gpointer user_data);


void GBusNameAcquired_Callback (GDBusConnection *connection,
                             const gchar *name,
                             gpointer user_data);

void GBusNameLost_Callback (GDBusConnection *connection,
                         const gchar *name,
                         gpointer user_data);


bool initDBusCommunicationForServer(void);

void *run(void* arg);

//客户端
void handleBoolChanged(TestServiceOrgExampleITestService *proxy, gboolean param, gpointer user_data);
void handleIntChanged(TestServiceOrgExampleITestService *proxy, gint32 param, gpointer user_data);
void handleDoubleChanged(TestServiceOrgExampleITestService *proxy, gdouble param, gpointer user_data);
void handleStringChanged(TestServiceOrgExampleITestService *proxy, const gchar* param, gpointer user_data);
void handleInfoChanged(TestServiceOrgExampleITestService *proxy, GVariant *param, gpointer user_data);


bool initDBusCommunicationForClient();

void show_menu();

std::string get_basename(const std::string& full_path);

bool send_file(const std::string& filename);

void clientLoop();