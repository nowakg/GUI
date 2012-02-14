/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_CAMERADEVICE_JUCEHEADER__
#define __JUCE_CAMERADEVICE_JUCEHEADER__

#if JUCE_USE_CAMERA || DOXYGEN


//==============================================================================
/**
    Controls any video capture devices that might be available.

    Use getAvailableDevices() to list the devices that are attached to the
    system, then call openDevice to open one for use. Once you have a CameraDevice
    object, you can get a viewer component from it, and use its methods to
    stream to a file or capture still-frames.
*/
class JUCE_API  CameraDevice
{
public:
    /** Destructor. */
    virtual ~CameraDevice();

    //==============================================================================
    /** Returns a list of the available cameras on this machine.

        You can open one of these devices by calling openDevice().
    */
    static const StringArray getAvailableDevices();

    /** Opens a camera device.

        The index parameter indicates which of the items returned by getAvailableDevices()
        to open.

        The size constraints allow the method to choose between different resolutions if
        the camera supports this. If the resolution cam't be specified (e.g. on the Mac)
        then these will be ignored.
    */
    static CameraDevice* openDevice (int deviceIndex,
                                     int minWidth = 128, int minHeight = 64,
                                     int maxWidth = 1024, int maxHeight = 768);

    //==============================================================================
    /** Returns the name of this device */
    const String getName() const                { return name; }

    /** Creates a component that can be used to display a preview of the
        video from this camera.
    */
    Component* createViewerComponent();

    //==============================================================================
    /** Starts recording video to the specified file.

        You should use getFileExtension() to find out the correct extension to
        use for your filename.

        If the file exists, it will be deleted before the recording starts.

        This method may not start recording instantly, so if you need to know the
        exact time at which the file begins, you can call getTimeOfFirstRecordedFrame()
        after the recording has finished.

        The quality parameter can be 0, 1, or 2, to indicate low, medium, or high. It may
        or may not be used, depending on the driver.
    */
    void startRecordingToFile (const File& file, int quality = 2);

    /** Stops recording, after a call to startRecordingToFile().
    */
    void stopRecording();

    /** Returns the file extension that should be used for the files
        that you pass to startRecordingToFile().

        This may be platform-specific, e.g. ".mov" or ".avi".
    */
    static const String getFileExtension();

    /** After calling stopRecording(), this method can be called to return the timestamp
        of the first frame that was written to the file.
    */
    const Time getTimeOfFirstRecordedFrame() const;

    //==============================================================================
    /**
        Receives callbacks with images from a CameraDevice.

        @see CameraDevice::addListener
    */
    class JUCE_API  Listener
    {
    public:
        Listener() {}
        virtual ~Listener() {}

        /** This method is called when a new image arrives.

            This may be called by any thread, so be careful about thread-safety,
            and make sure that you process the data as quickly as possible to
            avoid glitching!
        */
        virtual void imageReceived (const Image& image) = 0;
    };

    /** Adds a listener to receive images from the camera.

        Be very careful not to delete the listener without first removing it by calling
        removeListener().
    */
    void addListener (Listener* listenerToAdd);

    /** Removes a listener that was previously added with addListener().
    */
    void removeListener (Listener* listenerToRemove);


protected:
    /** @internal */
    CameraDevice (const String& name, int index);

private:
    void* internal;
    bool isRecording;
    String name;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CameraDevice);
};

/** This typedef is just for compatibility with old code - newer code should use the CameraDevice::Listener class directly. */
typedef CameraDevice::Listener CameraImageListener;


#endif
#endif   // __JUCE_CAMERADEVICE_JUCEHEADER__