# .. highlight:: python3
# .. default-domain:: py
#
# webcam_find_car.py
# ******************
# This module provides support code for driving a remote-controlled car based on feedback from a webcam. At its heart, this programs calls a user-supplied ``update`` function with a webcam image and provides routines to send commands to the car. It consists of the following functionality:
#
# - A main loop to grab an image from the webcam then call ``update``.
# - Image-processing code, :func:`find_car`, which determines the X, Y center of a given color (typically the color of the R/C car).
#
# .. _WebcamFindCar:
#
# Main loop
# =========
# The following class supplies code to grab an image from the webcam then call ``update``. In addition, it:
#
# - Creates a trackbar to allow easy of the :attr:`threshold` used by :func:`find_car`.
# - Handles mouse clicks by recording the color of the clicked pixel to use with :func:`find_car`.
#
# To do this, we rely on libraries which do most of the work. In particular, the following line provides access to the powerful image-processing routines provided by `OpenCV <docs.opencv.org>`_.
import cv2
# OpenCV_ relies heavily on `NumPy <numpy.scipy.org>`_ to manipulate arrays containing image data.
import numpy
# To communicate with the car, we simply send characters over the serial port using `PySerial <pyserial.sourceforge.net>`_.
import serial
# pickle lets the threshold and color persist across runs of this program.
import pickle as pickle
from math import sqrt, pi, sin, cos, atan2, copysign
from numpy import polyfit


# For testing, create a dummy Update class.
class Update_Mock(object):
    def __init__(self, ser):
        self.eco = Estimate_Car_Orientation(5, 10)

    # Stopping the car is easy: let it coast to a stop
    def stop(self, image):
        draw_str(image, (0, 15), "Stop")

    def drive(self, image, dist, dir_name, go_char, coast_char):
        draw_str(image, (0, 15), dir_name + (", dist = %.1f" % dist))

    # To drive foward or backward, alternate between driving and coasting.
    def forward(self, image, dist):
        self.drive(image, dist, "Forward", "C", " ")
    def backward(self, image, dist):
        self.drive(image, dist, "Backward", "G", " ")

    # To turn while driving, alternate the drive+turn with coast+turn.
    def forward_left(self, image, dist):
        self.drive(image, dist, "Forward and left", "D", "E")
    def forward_right(self, image, dist):
        self.drive(image, dist, "Forward and right", "B", "A")
    def backward_left(self, image, dist):
        self.drive(image, dist, "Backward and left", "F", "E")
    def backward_right(self, image, dist):
        self.drive(image, dist, "Backward and right", "H", "A")

    # :ref:`WebcamFindCar` calls this routine every time a webcam image is grabbed. Do all your processing here!
    def update(self, image, target_threshold, target_color, line_threshold, line_color, desired_xy, key):
        desired_x, desired_y = desired_xy
        # First, find the car in the given image. The ``actual_x`` and ``actual_y`` variables give the x, y location of the center of the car in the image.
        lab_image, final_image, (actual_x, actual_y), cont_area = find_car(image, target_color, target_threshold)
        draw_str(final_image, (0, 30), "Car area: %.1f" % cont_area)
        # Find a line / obstacle
        dist_image = distance_to_color(lab_image, final_image, line_color, line_threshold)
        # Display it if the line / obstacle was found
        if dist_image is not None:
            cv2.imshow("dist", dist_image / numpy.amax(dist_image))
        # Show the distance from the car's location to the nearest line / obstacle
        lobs_dist = get_dist_image(dist_image, actual_y, actual_x)
        if lobs_dist is not None:
            draw_str(final_image, (0, 45), "Dist to green: %.1f" % lobs_dist)
        # This specifies how close must the car be to the desired x, y coordinate for the car to stop.
        close_dist = 40
        # See if there's a drive destination or not.
        if desired_x < 0 or desired_y < 0:
            self.stop(final_image)
        else:
            # Determine how far it is from the center of the screen and draw that circle on the screen
            dist = sqrt( (desired_x - actual_x)**2 + (desired_y - actual_y)**2 )
            cv2.circle(final_image, (desired_x, desired_y), close_dist, (0,
 255, 255), 2)
             # Estimmate and plot the car's orientation
            car_angle = self.eco.estimate_car_orientation(actual_x, actual_y)
            draw_angle(final_image, (actual_x, actual_y), car_angle)
            if dist < close_dist:
                self.stop(final_image)
            else:
                # Determine the angle between the car's current position and its' desired position.
                target_angle = atan2(actual_y - desired_y, desired_x - actual_x)
                draw_angle(final_image, (actual_x, actual_y), target_angle)
                # Determine the angle at which to drive
                diff_angle = target_angle - car_angle
                if abs(diff_angle) <= pi/6.0:
                    self.forward(final_image, dist)
                elif diff_angle > 0.0 and diff_angle <= pi/2.0:
                    self.forward_left(final_image, dist)
                elif diff_angle < 0.0 and diff_angle >= -pi/2.0:
                    self.forward_right(final_image, dist)
                else:
                    self.stop(final_image)

        # Decide when to quit: return True to quit, False to keep running.
        return key != -1, final_image


# For debugging, or when the COM port isn't available, write data to the screen.
class Serial_Mock(object):
    def write(self, string):
        print(string)

    def isOpen(self):
        return True

    def close(self):
        pass

# This class implements all the main loop functionality. Simply instantiate it to use.
class Webcam_Find_Car(object):
    # To initialize the class, pick default values for the threshold and target_color, both used by  :func:`find_car`. The :attr:`update_func` is the user-supplied update routine. comm_port gives the serial port used to communicate with the car.
    def __init__(self, comm_port = None, webcam_index = 0, Update_class = Update_Mock):
        self.pickle_filename = 'webcam_find_car_defaults.pickle'
        try:
            with open(self.pickle_filename, 'rb') as f:
                (self.target_threshold, self.target_color, self.line_threshold, self.line_color) = pickle.load(f)
        except (IOError, ValueError):
            # Choose a default threshold if one can't be loaded from the last run of this program.
            self.target_threshold = 50
            self.line_threshold = self.target_threshold
            # Likewise, pick an initial target and line color.
            self.target_color = numpy.float32((34.0, 17.0, -47.0))
            self.line_color = self.target_color
        # Store the last x, y right mouse click. Indiate no initial choice with negative coordinates.
        self.last_rclick_coord = (-1, -1)

        # Open the serial port, if given
        if comm_port:
            self.ser = serial.Serial(port = comm_port - 1, baudrate = 115200)
        else:
            self.ser = Serial_Mock()
        # Initialize the update class. It needs an inst of ser to communiate with the car.
        update_inst = Update_class(self.ser)
        # All we need to call is the update method, so just save that.
        self.update_func = update_inst.update

        # Set up GUI
        cv2.namedWindow("final")
        cv2.namedWindow("dist")
        cv2.setMouseCallback("final", self.on_mouse)
        cv2.createTrackbar("car thold", "final", self.target_threshold, 100, self.on_trackbar_target_threshold)
        cv2.createTrackbar("line thold", "final", self.line_threshold, 100, self.on_trackbar_line_threshold)

        # Init video capture
        self.cap = cv2.VideoCapture(webcam_index)
        assert self.cap.isOpened()

# Grab an image then call ``update()`` until done.
    def main(self):
        # Using `waitKey <http://docs.opencv.org/modules/highgui/doc/user_interface.html#waitkey>`_, grab a frame then call update on it
        isDone = False
        while not isDone:
            key = cv2.waitKey(1)
            success_flag, self.image = self.cap.read()
            assert success_flag
            sz = self.image.shape
            self.image = cv2.resize(self.image, (int(sz[1]/2), int(sz[0]/2)))
            isDone = self.update(key)

        # Clean up
        if self.ser.isOpen():
            # Stop the car on exit
            self.ser.write(" ")
            self.ser.close()
        self.cap.release()
        cv2.destroyAllWindows()
        with open(self.pickle_filename, 'wb') as f:
            pickle.dump((self.target_threshold, self.target_color, self.line_threshold, self.line_color), f)

    def update(self, key = -1):
        isDone, final_image = self.update_func(self.image, self.target_threshold, self.target_color, self.line_threshold, self.line_color, self.last_rclick_coord, key)
        # Show the processed image
        draw_str(final_image, (5, final_image.shape[0] - 5), 'v268')
        cv2.imshow("final", final_image)
        return isDone

# Wrapping the code in a class makes access to class variables (such as :attr:`threshold`) from callbacks such as ``on_mouse`` below simpler.
    def on_mouse(self, event, x, y, flags, param):
        if event == cv2.EVENT_LBUTTONDOWN:
            if flags & cv2.EVENT_FLAG_CTRLKEY:
                print('line color')
                self.line_color = self.rgb_pixel_to_lab(x, y)
            else:
                print('target color')
                self.target_color = self.rgb_pixel_to_lab(x, y)
            self.update()
        elif event == cv2.EVENT_RBUTTONDOWN:
            self.last_rclick_coord = (x, y)

    # Sample an RGB pixel in an image and return its Lab equivalent.
    def rgb_pixel_to_lab(self, x, y):
        rgb_pixel_image = numpy.array([[self.image[y, x]]])
        target_pixel_image = im_to_lab(rgb_pixel_image)
        return target_pixel_image[0, 0]

    def on_trackbar_target_threshold(self, target_threshold):
        self.target_threshold = target_threshold
        self.update()

    def on_trackbar_line_threshold(self, line_threshold):
        self.line_threshold = line_threshold
        self.update()

# This routine places a string at the given location in an image. It was taken from openCV, in python2/samples/common.py.
def draw_str(dst, xy, s):
    x, y = xy
    cv2.putText(dst, s, (x + 1, y + 1), cv2.FONT_HERSHEY_PLAIN, 1.0, (0, 0, 0),
                thickness = 2)
    cv2.putText(dst, s, (x, y), cv2.FONT_HERSHEY_PLAIN, 1.0, (255, 255, 255))

def draw_angle(image, xy, angle):
    x, y = xy
    cv2.line(image, round_int((x, y)), round_int((15.0*cos(angle) + x, -15.0*sin(angle) + y)), (255, 255, 0), 2)

# Round f then convert it to an f. f can be a scalar or a tuple.
def round_int(f):
    if isinstance(f, tuple):
        return tuple(int(round(x)) for x in f)
    return int(round(f))

# Per the profiler, convering to the Lab colorspace takes most of the processing time. For the case of finding multiple colors, split this conversion out so it can be run once, rather than for each color to find.
def im_to_lab(image):
# Move from 8-bit color to 32-bit floating point to retain accuracy for the following calculations.
    float_image = image / numpy.float32(255.0)
# Using `cvtColor <http://docs.opencv.org/modules/imgproc/doc/miscellaneous_transformations.html#cvtcolor>`_, convert to the Lab_ color space, since distance in this space approximates human-perceived color differences.
    lab_image = cv2.cvtColor(float_image, cv2.COLOR_BGR2LAB)
    return lab_image

# This function finds a color blob (assumed to be the car), outlining it and returning its center.
def find_car(image, lab_color, thresh):
    lab_image = im_to_lab(image)
    contours = find_lab_color(lab_image, lab_color, thresh)
    cont_image, mass_center, cont_area = draw_car_contour(image, contours)
    return lab_image, cont_image, mass_center, cont_area

# This routine takes an image in the Lab color space, a color to find in that image, and a threshold around that color, then returns contours surrounding this color.
def find_lab_color(lab_image, color, thresh):
    assert(color.dtype == numpy.float32)
# Compute (image - target_color)^2, giving a Euclidian distance between the two.
    diff_image = lab_image - color
    normsq_image = numpy.sum(diff_image*diff_image, -1)
# `cv2.Threshold <http://docs.opencv.org/modules/imgproc/doc/miscellaneous_transformations.html#threshold>`_ the image to select only pixels close to the target color. Convert it from floating-point back to an 8-bit image, since the steps below require 8-bit input.
    (retval, thresh_image) = cv2.threshold(normsq_image, thresh**2.0, 255.0, cv2.THRESH_BINARY_INV)
    thresh_image = numpy.uint8(thresh_image)
# Perform a morphological open (`erode <http://docs.opencv.org/modules/imgproc/doc/filtering.html#cv2.erode>`_ then dilate), using `getStructuringElement <http://docs.opencv.org/modules/imgproc/doc/filtering.html#getstructuringelement>`_.
    sel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (3, 3))
    it = 2
    erode_image = cv2.erode(thresh_image, sel, iterations = it)
    open_image = cv2.dilate(erode_image, sel, iterations = it)
# Find the contours of the image, smooth them, and draw them
    output_image, contours0, hierarchy = cv2.findContours(open_image, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
#    contours = [cv2.approxPolyDP(cnt, 3, True) for cnt in contours0]
    contours = contours0
    return contours

# Given a contour, outline it and find its center.
def draw_car_contour(image, contours):
    if not contours:
        return image, (-1, -1), 0
# Select the largest-area contour, then compute its mass center
    cont_area = [cv2.contourArea(contour) for contour in contours]
    max_cont_area_index = numpy.argmax(cont_area)
    mass_center = compute_mass_center(contours[max_cont_area_index])
#    print(cont_area, max_cont_area_index, mass_center)
    cont_image = image.copy()
    cv2.drawContours(cont_image, contours, max_cont_area_index, (0, 0, 255), 3)
    # http://docs.opencv.org/modules/core/doc/drawing_functions.html#cv2.circle
    cv2.circle(cont_image, round_int(mass_center), 10, (0, 255, 255), -1)
    return cont_image, mass_center, cont_area[max_cont_area_index],

# Found this formula at    http://docs.opencv.org/modules/imgproc/doc/structural_analysis_and_shape_descriptors.html#moments
def compute_mass_center(contour):
    moments = cv2.moments(contour)
    m00 = moments['m00']
    if m00 == 0:
        return -1, -1
    m01 = moments['m01']
    m10 = moments['m10']
    x_m = m10/m00
    y_m = m01/m00
    return x_m, y_m

# Given a color and threhold, this routine computes a distance map from every pixel to the cloest found line
def distance_to_color(lab_image, display_image, lab_color, threshold):
    contours = find_lab_color(lab_image, lab_color, threshold)
    if not contours:
        return None
    # Outline all the found contours
    cv2.drawContours(display_image, contours, -1, (0, 255, 0), 3)
    # Zero all pixels in the found contours for use with the distance map
    dist_image_in = numpy.empty(lab_image.shape[0:-1], dtype = numpy.uint8)
    dist_image_in.fill(255)
    cv2.drawContours(dist_image_in, contours, -1, 0, cv2.FILLED)
    dist_image = cv2.distanceTransform(dist_image_in, cv2.DIST_L2, cv2.DIST_MASK_PRECISE)
    return dist_image


# Indexing an image produced by distance_to_color fails in two cases:
#
# - When the distance_to_color routine couldn't find any pixels of the correct color in the image, it returns None rather than a matrix
# - If the row or column coordinates lie outside the image. For example, draw_cor_contours returns an x, y location of (-1, -1) of the car can't be found.
#
# To simplify this interface, this routine checks for these cases and simply returns None, rather than throwing an exception.
def get_dist_image(dist_image, row, col):
    # If no obstacle was found, dist_image is None; exit now.
    if dist_image is None:
        return None
    # Otherwise, dist_image should be a 2-D array
    assert dist_image.ndim == 2
    # Check for out-of-bound access
    if (row < 0) or (col < 0) or (row > dist_image.shape[0]) or (col > dist_image.shape[1]):
        return None
    else:
        return dist_image[int(row), int(col)]


class Estimate_Car_Orientation(object):
    def __init__(self, num_history_points, dist_threshold):
        # Keep a list of the last few x, y locations of the car for use in esimating the car's orientation.
        self.actual_x_history = []
        self.actual_y_history = []

        # How many points to keep in the history
        self.num_history_points = num_history_points

        # When the history of points doesn't provide a good estimate, rely on the last valid estimate.
        self.last_orientation = pi/2.0

        # How many pixels of movement produce a valid orientation estimate
        self.dist_threshold = dist_threshold

    # Estimate the car's direction of travel based on fitting an arc of a circle to the last few x, y car locations. We assume that over this short sample time, there are no s-curves / direction changes to confuse this algorithm.
    def estimate_car_orientation(self, x, y):
        # First, add the current x, y coordinate to the end of the list, then drop the oldest coordinate from the beginning if the list is too long.
        self.actual_x_history.append(x)
        self.actual_y_history.append(y)
        length = len(self.actual_x_history)
        assert length == len(self.actual_y_history)
        if len(self.actual_x_history) > self.num_history_points:
            self.actual_x_history.pop(0)
            self.actual_y_history.pop(0)
            length -= 1
        # Is there enough distance between these points to trust this estimate? Sum up the distance beteen all points in the history then see if it's big enough.
        dist = 0
        for index in range(length - 1):
            dist += sqrt( (self.actual_x_history[index + 1] - self.actual_x_history[index])**2 +
                          (self.actual_y_history[index + 1] - self.actual_y_history[index])**2 )
        if dist < self.dist_threshold:
            return self.last_orientation
        # Next, fit the points. Polyfit stores coefficients of higher powers first, so an order-1 polynomial is :math:`C_0 x + C_1' -- slope then intercept.
        (slope, intercept), residuals, rank, singular_values, rcond = polyfit(self.actual_x_history, self.actual_y_history, 1, full = True)
        # Did we get a valid fit? A deficient rank indicates a vertical line.
        if rank < 2:
            self.last_orientation = pi/2.0
        # Otherwise, the slope gives the angle.  However, tan elementof [-pi/2, pi/2], so look at the vector connecting the first and last points to make this a four-quadrant result.
        else:
            self.last_orientation = atan2(
              copysign(slope, self.actual_y_history[0] - self.actual_y_history[-1]),
              copysign(1, self.actual_x_history[-1] - self.actual_x_history[0]))
        return self.last_orientation


def profile():
    import cProfile
    import pstats
    cProfile.run('main()', 'mainprof')
    p = pstats.Stats('mainprof')
    p.strip_dirs().sort_stats('time').print_stats(10)

def main():
    wfc = Webcam_Find_Car()
    wfc.main()

if __name__ == "__main__":
    main()
#    profile()


# .. _`grab an image`: http://docs.opencv.org/modules/highgui/doc/reading_and_writing_images_and_video.html#imread
# .. _show: http://docs.opencv.org/modules/highgui/doc/user_interface.html#imshow
# .. _Lab: http://en.wikipedia.org/wiki/Lab_color_space

# .. codelink:: cvtColor=http://docs.opencv.org/modules/imgproc/doc/miscellaneous_transformations.html#cvtcolor
