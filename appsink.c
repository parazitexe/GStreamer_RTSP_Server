#include <stdio.h>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

//build commmand
//gcc -Wall  -o "%e" "%f" `pkg-config --cflags --libs gstreamer-video-1.0 gstreamer-1.0 gstreamer-rtsp-server-1.0` -w



typedef struct {
  gboolean white;
  GstClockTime timestamp;
} MyContext;

/* called when we need to give data to appsrc */
static void need_data (GstElement * appsrc, guint unused, MyContext * ctx){
		printf("need_data \n");
  GstBuffer *buffer;
  guint size;
  GstFlowReturn ret;

  size = 385 * 288 * 2;

  buffer = gst_buffer_new_allocate (NULL, size, NULL);

  /* this makes the image black/white */
  gst_buffer_memset (buffer, 0, ctx->white ? 0xff : 0x0, size);

  ctx->white = !ctx->white;

  /* increment the timestamp every 1/2 second */
  GST_BUFFER_PTS (buffer) = ctx->timestamp;
  GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 2);
  ctx->timestamp += GST_BUFFER_DURATION (buffer);

  g_signal_emit_by_name (appsrc, "push-buffer", buffer, &ret);
}

/* called when a new media pipeline is constructed. We can query the
 * pipeline and configure our appsrc */
 
static void media_configure (GstRTSPMediaFactory * factory, GstRTSPMedia * media,  gpointer user_data){
	printf("media_configure \n");
	GstElement *element, *appsrc;
	MyContext *ctx;

	/* get the element used for providing the streams of the media */
	element = gst_rtsp_media_get_element (media);

	/* get our appsrc, we named it 'mysrc' with the name property */
	appsrc = gst_bin_get_by_name_recurse_up (GST_BIN (element), "mysrc");

	/* this instructs appsrc that we will be dealing with timed buffer */
	gst_util_set_object_arg (G_OBJECT (appsrc), "format", "time");
	/* configure the caps of the video */
	g_object_set (G_OBJECT (appsrc), "caps",
		gst_caps_new_simple ("video/x-raw",
			"format", G_TYPE_STRING, "RGB16",
			"width", G_TYPE_INT, 384,
			"height", G_TYPE_INT, 288,
			"framerate", GST_TYPE_FRACTION, 0, 1, NULL), NULL);

	ctx = g_new0 (MyContext, 1);
	ctx->white = FALSE;
	ctx->timestamp = 0;
	/* make sure ther datais freed when the media is gone */
	g_object_set_data_full (G_OBJECT (media), "my-extra-data", ctx,(GDestroyNotify) g_free);

	/* install the callback that will be called when a buffer is needed */
	g_signal_connect (appsrc, "need-data", (GCallback) need_data, ctx);
	gst_object_unref (appsrc);
	gst_object_unref (element);
}

void connected(GstRTSPServer *gstrtspserver,  GstRTSPClient *arg1, gpointer user_data){
	printf("cllient_connected \n");
}

int main (int argc, char *argv[]){
	GMainLoop 			*loop;
	
	//variables for create stream to localhost
	GstRTSPServer 		*server;
	GstRTSPMediaFactory *factory;
	GstRTSPMountPoints 	*mounts;

	//init gstreamer
	gst_init (&argc, &argv);
	
	server = gst_rtsp_server_new ();
	gst_rtsp_server_attach (server, NULL);
	factory = gst_rtsp_media_factory_new ();

	
	//gst_rtsp_media_factory_set_launch (factory, "( rtspsrc latency=0 max-ts-offset=0 location=rtsp://admin:admin1@192.168.100.222:554/cam/realmonitor?channel=1&subtype=1 ! rtph264depay !  rtph264pay pt=96 name=pay0 )");
	
	gst_rtsp_media_factory_set_launch (factory, "( appsrc name=mysrc ! videoconvert ! x264enc ! rtph264pay name=pay0 pt=96 )");	
	
	mounts = gst_rtsp_server_get_mount_points (server);
	gst_rtsp_mount_points_add_factory (mounts, "/local_rtsp_server", factory);
	//add handler for show conection
	g_signal_connect(G_OBJECT(server), "client-connected", (GCallback)connected, NULL);
	
	// notify when our media is ready, This is called whenever someone asks for
	// the media and a new pipeline with our appsrc is created
	g_signal_connect (factory, "media-configure", (GCallback) media_configure, NULL);
	

	/* start serving */
	loop = g_main_loop_new (NULL, FALSE);
	g_main_loop_run (loop);
	
	g_object_unref (mounts);
     

  return 0;
}
