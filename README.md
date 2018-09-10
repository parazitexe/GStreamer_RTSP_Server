GStreamer_RTSP_Server


//build commmand Geany
```
gcc -Wall  -o "%e" "%f" `pkg-config --cflags --libs gstreamer-video-1.0 gstreamer-1.0 gstreamer-rtsp-server-1.0` -w -lgstrtsp-1.0
```

//build commmand for CLI
```
gcc -Wall  -o server server.c `pkg-config --cflags --libs gstreamer-video-1.0 gstreamer-1.0 gstreamer-rtsp-server-1.0` -w -lgstrtsp-1.0
```


//install GstRTSPServer
```
sudo apt-get install libgstrtspserver-1.0-dev
```
