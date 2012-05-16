// Process an image to find the serial number of
// barcodes. Needs OpenCV and zbar.

// Include header files
#include "cv.h"
#include "highgui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>

#include "zbar.h"

// Create memory for calculations
static CvMemStorage* storage = 0;

// Function prototype for detecting and drawing an object from an image
void process_and_draw( IplImage* image );

// Main function, defines the entry point for the program.
int main( int argc, char** argv )
{
	
    // Structure for getting video from camera or avi
    CvCapture* capture = 0;
	
    // Images to capture the frame from video or camera or from file
    IplImage *frame, *frame_copy = 0;
	
    // Allocate the memory storage
    storage = cvCreateMemStorage(0);
    capture = cvCaptureFromCAM(0);
	
    // Create a new named window with title: result
    cvNamedWindow( "result", 1 );
	cvNamedWindow( "rect", 2 );
	
    // Find if the capture is loaded successfully or not.
	
    // If loaded succesfully, then:
    if( capture )
    {
        // Capture from the camera.
        for(;;)
        {
            // Capture the frame and load it in IplImage
            if( !cvGrabFrame( capture ))
                break;
            frame = cvRetrieveFrame( capture , 0);
			
            // If the frame does not exist, quit the loop
            if( !frame )
                break;
            
            // Allocate framecopy as the same size of the frame
            if( !frame_copy )
                frame_copy = cvCreateImage( cvSize(frame->width,frame->height),
										   IPL_DEPTH_8U, frame->nChannels );
			
            // Check the origin of image. If top left, copy the image frame to frame_copy. 
            if( frame->origin == IPL_ORIGIN_TL )
                cvCopy( frame, frame_copy, 0 );
            // Else flip and copy the image
            else
                cvFlip( frame, frame_copy, 0 );
            
            // Call the function to process the image and draw it
            process_and_draw( frame_copy );
			
            // Wait for a while before proceeding to the next frame
            if( cvWaitKey( 10 ) >= 0 )
                break;
        }
		
        // Release the images, and capture memory
        cvReleaseImage( &frame_copy );
        cvReleaseCapture( &capture );
    }
	
    // If the capture is not loaded succesfully, then:
    else
    {
		printf(stderr, "ERROR: capture not loaded successfully");
    }
    
    // Destroy the window previously created with filename: "result"
    cvDestroyWindow("result");
	
    // return 0 to indicate successfull execution of the program
    return 0;
}

// Gets the Barcode serial number from the image
int GetCode(IplImage *img) {
	
    /* create a reader */
    zbar_image_scanner_t * scanner;
	scanner = zbar_image_scanner_create();
	
    /* configure the reader */
    zbar_image_scanner_set_config(scanner, ZBAR_EAN13, ZBAR_CFG_ENABLE, 1);
	
    /* obtain image data */
	CvSize r = cvGetSize(img);	
    int width = r.width, height = r.height;
	void* raw = (uchar *)(img->imageData);
	
    /* wrap image data */
    zbar_image_t *image = zbar_image_create();
    zbar_image_set_format(image, *(int*)"Y800");
    zbar_image_set_size(image, width, height);
    zbar_image_set_data(image, raw, width * height, zbar_image_free_data);
	
    /* scan the image for barcodes */
    int n = zbar_scan_image(scanner, image);
	
    /* extract results */
    const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
    for(; symbol; symbol = zbar_symbol_next(symbol)) {
        /* do something useful with results */
        zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
        const char *data = zbar_symbol_get_data(symbol);
        printf("%s symbol \"%s\"\n",
               zbar_get_symbol_name(typ), data);
    }
	/* clean up */
    // zbar_image_destroy(image);
	
	return n;
	
}

// Function to detect and draw any faces that is present in an image
void process_and_draw( IplImage* img )
{
    int scale = 1;
	
    // Create a new image based on the input image
    IplImage* temp = cvCreateImage( cvSize(img->width/scale,img->height/scale), 8, 3 );
	IplImage* grey = cvCreateImage( cvGetSize( img ) , 8 , 1);
	IplImage* edges = cvCreateImage( cvGetSize( img ) , 8 , 1);
	
	cvCvtColor( img , grey , CV_BGR2GRAY );	
	// cvSobel(grey,edges,1,1,3);
	cvCanny(grey, edges, 1.2, 10, 5);

	// Draw a rectangle for side_percentage% of the image
	CvSize r = cvGetSize(img);
	CvPoint origin, corner1, corner2;
	
	double side_percentage = 0.4;
	double alpha = (1 - side_percentage) / 2;
	
	origin.x = r.width * alpha;
	origin.y = r.height * alpha;
	
	r.width = r.width * side_percentage;
	r.height = r.height * side_percentage;
	
	corner1 = origin;
	corner2.x = origin.x + r.width;
	corner2.y = origin.y + r.height;	
	
	cvRectangle( grey, corner1, corner2, CV_RGB(255,0,0), 3, 8, 0 );
	
	IplImage* temp2 = cvCreateImage( cvSize(r.width,r.height), 8, 1 );
	cvSetImageROI( grey, cvRect(corner1.x, corner1.y, r.width, r.height));
	
	cvCopy(grey, temp2, NULL);
	cvResetImageROI(grey);

	int code = GetCode(temp2);
	printf("%i", code);
	
    // Clear the memory storage which was used before
    cvClearMemStorage( storage );
	
    // Show the image in the window named "result"
    cvShowImage( "result", grey );
	cvShowImage( "rect", temp2 );
	
    // Release the temp image created.
	cvReleaseImage( &temp );
    cvReleaseImage( &temp2 );
	cvReleaseImage( &grey );
	cvReleaseImage( &edges );
}

