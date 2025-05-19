hello: hello.c
	cc -g hello.c -o hello $$(pkg-config --cflags --libs libavformat libavcodec libavutil)

display: display_video.c
	cc -g $< -o $@ $$(pkg-config --cflags --libs libavformat libavcodec libavutil) $$(pkg-config --cflags --libs sdl2)

%: %.c
	cc -g $< -o $@ $$(pkg-config --cflags --libs libavformat libavcodec libavutil) $$(pkg-config --cflags --libs sdl2)

clean:
	rm -vrf hello frame-*.pgm *.dSYM \
		display_video