#include <stdio.h>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

#include <gst/rtsp/gstrtspurl.h>
#include <gst/rtsp/gstrtspconnection.h>

//build commmand
//gcc -Wall  -o "%e" "%f" `pkg-config --cflags --libs gstreamer-video-1.0 gstreamer-1.0 gstreamer-rtsp-server-1.0` -w -lgstrtsp-1.0


//install GstRTSPServer
//sudo apt-get install libgstrtspserver-1.0-dev

enum connection_status{
	OFFLINE,
	ONLINE
};

enum server_stream_status{
	RE_STREAM_WORK,
	RE_STREAM_STOP
};

struct _server_status{
	gint	connection;
	gint	stream;
} server_status;

const char* location 					=  "rtsp://admin:admin1@192.168.100.222:554/cam/realmonitor?channel=1&subtype=1";
const int* check_connection_timeout 	=  3;


void connected(GstRTSPServer *gstrtspserver,  GstRTSPClient *arg1, gpointer user_data){
	printf("cllient_connected \n");
}

static gboolean check_connection();

void run_server();

int main (int argc, char *argv[]){
	
	GMainLoop 			*loop;
	
	server_status.connection 	= OFFLINE;
	server_status.stream 		= RE_STREAM_STOP;
	
	//init gstreamer
	gst_init (&argc, &argv);
	
	//first "initial" launch of check connection for run server
	check_connection();
	//run requests for check RTSP SERVER connection status and reconnect if connection fail
	g_timeout_add_seconds ( check_connection_timeout, (GSourceFunc)check_connection, NULL);
	

	/* start serving */
	loop = g_main_loop_new (NULL, FALSE);
	g_main_loop_run (loop);

	return 0;
}

//run_RTSP_server
void run_server(){
	printf("server_run \n");
	
	//variables for create stream to localhost
	GstRTSPServer 		*server;
	GstRTSPMediaFactory *factory;
	GstRTSPMountPoints 	*mounts;
	
	server = gst_rtsp_server_new ();
	gst_rtsp_server_attach (server, NULL);
	factory = gst_rtsp_media_factory_new ();
	
	
	char factory_launch_string[1000];
	//sprintf(factory_launch_string, "( rtspsrc  latency=0 max-ts-offset=0 location=%s%s", location, " ! rtph264depay ! rtph264pay pt=96 name=pay0  )");
	sprintf(factory_launch_string, "( videotestsrc ! rtph264depay ! rtph264pay pt=96 name=pay0  )");
	
	gst_rtsp_media_factory_set_launch (factory, factory_launch_string);
	mounts = gst_rtsp_server_get_mount_points (server);
	gst_rtsp_mount_points_add_factory (mounts, "/local_rtsp_server", factory );
	//add handler for show conection of clients
	g_signal_connect(G_OBJECT(server), "client-connected", (GCallback)connected, NULL );
	
	g_object_unref (mounts);
	server_status.stream = RE_STREAM_WORK;
	printf("server_run_successfully \n");
}

//check connection to RTSP
static gboolean check_connection(){
	printf("check_connection to RTSP SERVER \n");
	
	const gchar 		*urlstr = location;
	
	GstRTSPUrl 			*url;
	GstRTSPConnection 	*conn;
	GTimeVal 			timeout;
	GstRTSPMessage 		*response;
	
	timeout.tv_sec = 	check_connection_timeout;
	timeout.tv_usec = 	0;
	
	
	gst_rtsp_url_parse( urlstr, &url );
	gst_rtsp_connection_create (url, &conn);
	
	
	if(gst_rtsp_connection_connect_with_response (conn, &timeout, response) == GST_RTSP_OK){
		gst_rtsp_connection_close(conn);
		server_status.connection 	= ONLINE;
	}else{
		server_status.connection 	= OFFLINE;
		server_status.stream 		= RE_STREAM_STOP;
	}
	
	printf ("server connection \t - %s \n",	( server_status.connection == ONLINE) ? "ONLINE" : "OFFLINE" );
	printf ("server stream \t\t - %s \n", 	( server_status.stream == RE_STREAM_STOP ) ? "RE_STREAM_STOP" : "RE_STREAM_PLAY" );
	
	//if connection exist but server not  run yet or stoped
	if(server_status.connection == ONLINE && server_status.stream == RE_STREAM_STOP ){
		run_server();
	}
	
	return TRUE;
}
