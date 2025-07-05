### Application to test jpeg files on the Sony psp using the built in ME lib

Image used: https://en.wikipedia.org/wiki/JPEG#/media/File:Felis_silvestris_silvestris_small_gradual_decrease_of_quality_-_JPEG_compression.jpg <br>

Requires: https://github.com/raylib4Consoles/raylib4Psp <br>

Command used for converting chroma subsampling: `ffmpeg -i fileIn.jpg -vf format=yuv420p,scale=400x400 file.jpg`

Results:
- [ ] yuv444p
- [ ] yuvj444p
- [ ] yuv422p
- [ ] yuvj422p
- [X] yuv420p
- [X] yuvj420p
<br>
<br>
If using this app as a sample for decoding remember error checking on the sceJpeg functions!
