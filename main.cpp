#include <stdio.h>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>


class AppServer{
public:
	AppServer(){
		
		printf("AppServer Constructor \n");

		loop = g_main_loop_new (NULL, FALSE);
		
		server = gst_rtsp_server_new ();
		gst_rtsp_server_attach (server, NULL);

		factory = gst_rtsp_media_factory_new ();
		gst_rtsp_media_factory_set_launch (factory, "( rtspsrc protocols=tcp location=rtsp://admin:admin1@192.168.100.222:554/cam/realmonitor?channel=1&subtype=1 ! rtph264depay ! avdec_h264 ! x264enc ! rtph264pay pt=96 name=pay0 )");
		mounts = gst_rtsp_server_get_mount_points (server);
		gst_rtsp_mount_points_add_factory (mounts, "/test", factory);
		g_signal_connect(G_OBJECT(server), "client-connected", (GCallback)connected, NULL);

		g_main_loop_run (loop);
			
	}
	~AppServer(){
		printf("AppServer Destructor \n");
	}
	static gboolean connected(GstRTSPServer *gstrtspserver,  GstRTSPClient *arg1, gpointer user_data){
		printf("cllient_connected \n");
		return TRUE;
	}
private:
	GstRTSPServer *server;
	GMainLoop *loop;
	GstRTSPMediaFactory *factory;
	GstRTSPMountPoints *mounts;
};


int main (int argc, char *argv[]){
	
	gst_init (&argc, &argv);
	
	AppServer server;
	printf("server run \n");
	
	
	return 0;
}
