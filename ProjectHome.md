QRCore is a C library that finds QR Code corners in a efficient way.
Currently, it does:

  * image binarization with local thresholds
  * binary image processing to find all 4 corners from a single QR code in the image

| **Raw image** | **Processed image** (corners are red) |
|:--------------|:--------------------------------------|
| ![http://qrcore.googlecode.com/svn/trunk/qr_core/test/data/qrcode-3/01.png](http://qrcore.googlecode.com/svn/trunk/qr_core/test/data/qrcode-3/01.png) | ![http://qrcore.googlecode.com/svn/trunk/qr_core/test/data/qrcode-3/01_out.png](http://qrcore.googlecode.com/svn/trunk/qr_core/test/data/qrcode-3/01_out.png) |