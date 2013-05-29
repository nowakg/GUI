/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "LfpDisplayCanvas.h"

#include <math.h>

LfpDisplayCanvas::LfpDisplayCanvas(LfpDisplayNode* processor_) :
    screenBufferIndex(0), timebase(1.0f), displayGain(1.0f),   timeOffset(0.0f),
    processor(processor_),
     displayBufferIndex(0)
{

    nChans = processor->getNumInputs();
    sampleRate = processor->getSampleRate();
    std::cout << "Setting num inputs on LfpDisplayCanvas to " << nChans << std::endl;

    displayBuffer = processor->getDisplayBufferAddress();
    displayBufferSize = displayBuffer->getNumSamples();
    std::cout << "Setting displayBufferSize on LfpDisplayCanvas to " << displayBufferSize << std::endl;

    screenBuffer = new AudioSampleBuffer(MAX_N_CHAN, MAX_N_SAMP);

    viewport = new Viewport();
    lfpDisplay = new LfpDisplay(this, viewport);
    timescale = new LfpTimescale(this);

    timescale->setTimebase(timebase);

    viewport->setViewedComponent(lfpDisplay, false);
    viewport->setScrollBarsShown(true, false);

    scrollBarThickness = viewport->getScrollBarThickness();


    //viewport->getVerticalScrollBar()->addListener(this->scrollBarMoved(viewport->getVerticalScrollBar(), 1.0));



    addAndMakeVisible(viewport);
    addAndMakeVisible(timescale);

	voltageRanges.add("50");
    voltageRanges.add("100");
    voltageRanges.add("500");
    voltageRanges.add("1000");
    voltageRanges.add("2000");
    voltageRanges.add("5000");

    timebases.add("0.2");
    timebases.add("0.5");
    timebases.add("1.0");
    timebases.add("2.0");
    timebases.add("5.0");
    timebases.add("10.0");

    
    spreads.add("10");
    spreads.add("20");
    spreads.add("30");
    spreads.add("40");
    spreads.add("50");
    spreads.add("60");


    rangeSelection = new ComboBox("Voltage range");
    rangeSelection->addItemList(voltageRanges, 1);
    rangeSelection->setSelectedId(4,false);
    rangeSelection->addListener(this);
    addAndMakeVisible(rangeSelection);

    timebaseSelection = new ComboBox("Timebase");
    timebaseSelection->addItemList(timebases, 1);
    timebaseSelection->setSelectedId(3,false);
    timebaseSelection->addListener(this);
    addAndMakeVisible(timebaseSelection);


    spreadSelection = new ComboBox("Spread");
    spreadSelection->addItemList(spreads, 1);
    spreadSelection->setSelectedId(5,false);
    spreadSelection->addListener(this);
    addAndMakeVisible(spreadSelection);


    lfpDisplay->setNumChannels(nChans);
    lfpDisplay->setRange(1000.0f);

}

LfpDisplayCanvas::~LfpDisplayCanvas()
{

    deleteAndZero(screenBuffer);
}

void LfpDisplayCanvas::resized()
{

    timescale->setBounds(leftmargin,0,getWidth()-scrollBarThickness,30);
    viewport->setBounds(0,30,getWidth(),getHeight()-90);

    lfpDisplay->setBounds(0,0,getWidth()-scrollBarThickness, getChannelHeight()*nChans);

    rangeSelection->setBounds(5,getHeight()-30,100,25);
    timebaseSelection->setBounds(175,getHeight()-30,100,25);
    spreadSelection->setBounds(345,getHeight()-30,100,25);

   // std::cout << "Canvas thinks LfpDisplay should be this high: " 
    //	<< lfpDisplay->getTotalHeight() << std::endl;

}

void LfpDisplayCanvas::beginAnimation()
{
    std::cout << "Beginning animation." << std::endl;

    displayBufferSize = displayBuffer->getNumSamples();

    screenBufferIndex = 0;

    startCallbacks();
}

void LfpDisplayCanvas::endAnimation()
{
    std::cout << "Ending animation." << std::endl;

    stopCallbacks();
}

void LfpDisplayCanvas::update()
{
    nChans = processor->getNumInputs();
    sampleRate = processor->getSampleRate();

    std::cout << "Setting num inputs on LfpDisplayCanvas to " << nChans << std::endl;

    refreshScreenBuffer();

    lfpDisplay->setNumChannels(nChans);
    lfpDisplay->setBounds(0,0,getWidth()-scrollBarThickness*2, lfpDisplay->getTotalHeight());


    //repaint();

}

void LfpDisplayCanvas::comboBoxChanged(ComboBox* cb)
{

    if (cb == timebaseSelection)
    {
        timebase = timebases[cb->getSelectedId()-1].getFloatValue();
    }
    else if (cb == rangeSelection)
    {
        lfpDisplay->setRange(voltageRanges[cb->getSelectedId()-1].getFloatValue());
        //std::cout << "Setting range to " << voltageRanges[cb->getSelectedId()-1].getFloatValue() << std::endl;
    }
    else if (cb == spreadSelection)
    {
         //spread = spreads[cb->getSelectedId()-1].getFloatValue();
         lfpDisplay->setChannelHeight(spreads[cb->getSelectedId()-1].getIntValue());
         //lfpDisplay->resized();
         resized();
         //std::cout << "Setting spread to " << spreads[cb->getSelectedId()-1].getFloatValue() << std::endl;
    }

    timescale->setTimebase(timebase);
}




int LfpDisplayCanvas::getChannelHeight()
{
	return spreads[spreadSelection->getSelectedId()-1].getIntValue();

}


void LfpDisplayCanvas::setParameter(int param, float val)
{
    // if (param == 0)
    // {
    //     timebase = val;
    //     refreshScreenBuffer();
    // }
    // else
    // {
    //     displayGain = val; //* 0.0001f;
    // }

    // repaint();
}

void LfpDisplayCanvas::refreshState()
{
    // called when the component's tab becomes visible again
    displayBufferIndex = processor->getDisplayBufferIndex();
    screenBufferIndex = 0;

}

void LfpDisplayCanvas::refreshScreenBuffer()
{

    screenBufferIndex = 0;

    screenBuffer->clear();

    // int w = lfpDisplay->getWidth();
    // //std::cout << "Refreshing buffer size to " << w << "pixels." << std::endl;

    // for (int i = 0; i < w; i++)
    // {
    // 	float x = float(i);

    // 	for (int n = 0; n < nChans; n++)
    // 	{
    // 		waves[n][i*2] = x;
    // 		waves[n][i*2+1] = 0.5f; // line in center of display
    // 	}
    // }

}

void LfpDisplayCanvas::updateScreenBuffer()
{


    // copy new samples from the displayBuffer into the screenBuffer (waves)
    int maxSamples = lfpDisplay->getWidth();

    if (screenBufferIndex>=maxSamples) // wrap around if we reached right edge before
        screenBufferIndex=leftmargin;

    lastScreenBufferIndex = screenBufferIndex;


    int index = processor->getDisplayBufferIndex();

    int nSamples =  index - displayBufferIndex; // N new samples to be addeddisplayBufferIndex

    if (nSamples < 0) // buffer has reset to 0
    {
        nSamples = (displayBufferSize - displayBufferIndex) + index;
    }

    float ratio = sampleRate * timebase / float(getWidth());

    // this number is crucial: converting from samples to values (in px) for the screen buffer
    int valuesNeeded = (int) float(nSamples) / ratio;


    if ( screenBufferIndex + valuesNeeded > maxSamples) // crop number of samples to fit cavas width
    {
            valuesNeeded = maxSamples - screenBufferIndex;
    }

    float subSampleOffset = 0.0;
    int nextPos = (displayBufferIndex ) % displayBufferSize; //  position next to displayBufferIndex in display buffer to copy from

    if (valuesNeeded > 0 && valuesNeeded < 1000)
    {

        for (int i = 0; i < valuesNeeded; i++) // also fill one extra sample for line drawing interpolation to match across draws
        {
            float gain = 1.0;
            float alpha = (float) subSampleOffset;
            float invAlpha = 1.0f - alpha;

            screenBuffer->clear(screenBufferIndex, 1);

            //if (displayBufferIndex<=index && nextPos<=index){
            for (int channel = 0; channel < nChans; channel++)
            {

                screenBuffer->addFrom(channel, // destChannel
                                      screenBufferIndex, // destStartSample
                                      displayBuffer->getSampleData(channel, displayBufferIndex), // source
                                      1, // numSamples
                                      invAlpha*gain); // gain

                screenBuffer->addFrom(channel, // destChannel
                                      screenBufferIndex, // destStartSample
                                      displayBuffer->getSampleData(channel, nextPos), // source
                                      1, // numSamples
                                      alpha*gain); // gain

                //waves[channel][screenBufferIndex*2+1] =
                //	*(displayBuffer->getSampleData(channel, displayBufferIndex))*invAlpha*gain*displayGain;

                //waves[channel][screenBufferIndex*2+1] +=
                //	*(displayBuffer->getSampleData(channel, nextPos))*alpha*gain*displayGain;

                //waves[channel][screenBufferIndex*2+1] += 0.5f; // to center in viewport

            };
            //};
            //// now do the event channel
            ////	waves[nChans][screenBufferIndex*2+1] =
            //		*(displayBuffer->getSampleData(nChans, displayBufferIndex));


            subSampleOffset += ratio;

            while (subSampleOffset >= 1.0)
            {
                if (++displayBufferIndex > displayBufferSize)
                    displayBufferIndex = 0;

                nextPos = (displayBufferIndex + 1) % displayBufferSize;
                subSampleOffset -= 1.0;
            }

            screenBufferIndex++;
           // screenBufferIndex %= maxSamples;

        }
                

    }
    else
    {
        //std::cout << "Skip." << std::endl;
    }
}

float LfpDisplayCanvas::getXCoord(int chan, int samp)
{
    return samp;
}

float LfpDisplayCanvas::getYCoord(int chan, int samp)
{
    return *screenBuffer->getSampleData(chan, samp);
}

void LfpDisplayCanvas::paint(Graphics& g)
{

    //std::cout << "Painting" << std::endl;
    g.setColour(Colour(0,18,43)); //background color
    g.fillRect(0, 0, getWidth(), getHeight());

    g.setColour(Colour(40,40,40));

    int w = getWidth()-scrollBarThickness;

    for (int i = 1; i < 10; i++)
    {
        if (i == 5)
            g.drawLine(w/10*i,0,w/10*i,getHeight()-60,3.0f);
        else
            g.drawLine(w/10*i,0,w/10*i,getHeight()-60,1.0f);
    }

    g.drawLine(0,getHeight()-60,getWidth(),getHeight()-60,3.0f);

    g.setFont(Font("Default", 16, Font::plain));

    g.setColour(Colour(100,100,100));

    g.drawText("Voltage range (uV)",5,getHeight()-55,300,20,Justification::left, false);
    g.drawText("Timebase (s)",175,getHeight()-55,300,20,Justification::left, false);
    g.drawText("Spread (px)",345,getHeight()-55,300,20,Justification::left, false);

}

void LfpDisplayCanvas::refresh() 
{
    updateScreenBuffer();

    lfpDisplay->refresh(); // redraws only the new part of the screen buffer

    //getPeer()->performAnyPendingRepaintsNow();

}

void LfpDisplayCanvas::saveVisualizerParameters(XmlElement* xml)
{

    XmlElement* xmlNode = xml->createNewChildElement("LFPDISPLAY");


    xmlNode->setAttribute("Range",rangeSelection->getSelectedId());
    xmlNode->setAttribute("Timebase",timebaseSelection->getSelectedId());
    xmlNode->setAttribute("Spread",spreadSelection->getSelectedId());

    xmlNode->setAttribute("ScrollX",viewport->getViewPositionX());
    xmlNode->setAttribute("ScrollY",viewport->getViewPositionY());
}


void LfpDisplayCanvas::loadVisualizerParameters(XmlElement* xml)
{
    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("LFPDISPLAY"))
        {
            rangeSelection->setSelectedId(xmlNode->getIntAttribute("Range"));
            timebaseSelection->setSelectedId(xmlNode->getIntAttribute("Timebase"));
            spreadSelection->setSelectedId(xmlNode->getIntAttribute("Spread"));

            viewport->setViewPosition(xmlNode->getIntAttribute("ScrollX"),
                                      xmlNode->getIntAttribute("ScrollY"));
        }
    }

}


// -------------------------------------------------------------

LfpTimescale::LfpTimescale(LfpDisplayCanvas* c) : canvas(c)
{

    font = Font("Default", 16, Font::plain);
}

LfpTimescale::~LfpTimescale()
{

}

void LfpTimescale::paint(Graphics& g)
{

    g.setGradientFill(ColourGradient(Colour(50,50,50),0,0,
                                     Colour(25,25,25),0,getHeight(),
                                     false));

    g.fillAll();

    g.setColour(Colours::black);

    g.drawLine(0,getHeight(),getWidth(),getHeight());

    g.setFont(font);

    g.setColour(Colour(100,100,100));

    g.drawText("ms:",5,0,100,getHeight(),Justification::left, false);

    for (int i = 1; i < 10; i++)
    {
        if (i == 5)
            g.drawLine(getWidth()/10*i,0,getWidth()/10*i,getHeight(),3.0f);
        else
            g.drawLine(getWidth()/10*i,0,getWidth()/10*i,getHeight(),1.0f);

        g.drawText(labels[i-1],getWidth()/10*i+3,0,100,getHeight(),Justification::left, false);
    }

}

void LfpTimescale::setTimebase(float t)
{
    timebase = t;

    labels.clear();

    for (float i = 1.0f; i < 10.0; i++)
    {
        String labelString = String(timebase/10.0f*1000.0f*i);

        labels.add(labelString.substring(0,4));
    }

    repaint();

}


// ---------------------------------------------------------------

LfpDisplay::LfpDisplay(LfpDisplayCanvas* c, Viewport* v) :
    canvas(c), viewport(v), range(1000.0f)
{

    totalHeight = 0;

    addMouseListener(this, true);

    // hue cycle
    //for (int i = 0; i < 15; i++)
    //{
    //    channelColours.add(Colour(float(sin((3.14/2)*(float(i)/15))),float(1.0),float(1),float(1.0)));
    //}




    //hand built palette
    channelColours.add(Colour(224,185,36));
    channelColours.add(Colour(214,210,182));
    channelColours.add(Colour(243,119,33));
    channelColours.add(Colour(186,157,168));
    channelColours.add(Colour(237,37,36));
    channelColours.add(Colour(179,122,79));
    channelColours.add(Colour(217,46,171));
    channelColours.add(Colour(217, 139,196));
    channelColours.add(Colour(101,31,255));
    channelColours.add(Colour(141,111,181));
    channelColours.add(Colour(48,117,255));
    channelColours.add(Colour(184,198,224));
    channelColours.add(Colour(116,227,156));
    channelColours.add(Colour(150,158,155));
    channelColours.add(Colour(82,173,0));
    channelColours.add(Colour(125,99,32));

}

LfpDisplay::~LfpDisplay()
{
    deleteAllChildren();
}

void LfpDisplay::setNumChannels(int numChannels)
{
    numChans = numChannels;

    deleteAllChildren();

    channels.clear();

    totalHeight = 0;

    for (int i = 0; i < numChans; i++)
    {

        //std::cout << "Adding new channel display." << std::endl;

        LfpChannelDisplay* lfpChan = new LfpChannelDisplay(canvas, i);

        lfpChan->setColour(channelColours[i % channelColours.size()]);
        lfpChan->setRange(range);
        lfpChan->setChannelHeight(canvas->getChannelHeight());

        addAndMakeVisible(lfpChan);

        channels.add(lfpChan);

        totalHeight += lfpChan->getChannelHeight();

    }

}

int LfpDisplay::getTotalHeight()
{
	return totalHeight;
}

void LfpDisplay::resized()
{

    int totalHeight = 0;

    for (int i = 0; i < numChans; i++)
    {

        LfpChannelDisplay* disp = channels[i];

        disp->setBounds(0,
                totalHeight-disp->getChannelOverlap()/2,
                getWidth(),
                disp->getChannelHeight()+disp->getChannelOverlap());

        totalHeight += disp->getChannelHeight();

    }

    canvas->fullredraw=true;//issue full redraw 

   // std::cout << "Total height: " << totalHeight << std::endl;

}

void LfpDisplay::paint(Graphics& g)
{

}

void LfpDisplay::refresh()
{


    int topBorder = viewport->getViewPositionY();
    int bottomBorder = viewport->getViewHeight() + topBorder;

    // ensure that only visible channels are redrawn
    for (int i = 0; i < numChans; i++)
    {

        int componentTop = getChildComponent(i)->getY();
        int componentBottom = getChildComponent(i)->getHeight() + componentTop;

        if ((topBorder <= componentBottom && bottomBorder >= componentTop))
        {
            if (canvas->fullredraw){
                channels[i]->fullredraw=true;
                getChildComponent(i)->repaint();
            } else{
                getChildComponent(i)->repaint( canvas->lastScreenBufferIndex-2, 0, (canvas->screenBufferIndex-canvas->lastScreenBufferIndex)+3, getChildComponent(i)->getHeight() ); //repaint only the updated portion
                // we redraw from -2 to +1 relative to the real redraw window, the -3 makes sure that the lines join nicely, and the +1 draws the vertical update line
            }
            //std::cout << i << std::endl;
        }

    }
        canvas->fullredraw=false;
}

void LfpDisplay::setRange(float r)
{

    range = r;

    for (int i = 0; i < numChans; i++)
    {

        channels[i]->setRange(range);

    }

}

void LfpDisplay::setChannelHeight(int r)
{

    for (int i = 0; i < numChans; i++)
    {
        channels[i]->setChannelHeight(r);
    }

    resized();

}


 void LfpDisplay::mouseWheelMove(const MouseEvent&  e, const MouseWheelDetails&   wheel )   {
    
    canvas->fullredraw=true;//issue full redraw 

    //  passes the event up to the viewport so the screen scrolls
    if (viewport != nullptr && e.eventComponent == this) // passes only if it's not a listening event
        viewport->mouseWheelMove(e.getEventRelativeTo(canvas), wheel);
 }


void LfpDisplay::mouseDown(const MouseEvent& event)
{
    //int x = event.getMouseDownX();
    //int y = event.getMouseDownY();

    //std::cout << "Mouse down at " << x << ", " << y << std::endl;


    for (int n = 0; n < numChans; n++)
    {
        channels[n]->deselect();
    }

    LfpChannelDisplay* lcd = (LfpChannelDisplay*) event.eventComponent;

    lcd->select();

    canvas->fullredraw=true;//issue full redraw 

    //repaint();

}

// ------------------------------------------------------------------

LfpChannelDisplay::LfpChannelDisplay(LfpDisplayCanvas* c, int channelNumber) :
    canvas(c), isSelected(false), chan(channelNumber), channelHeight(40), channelOverlap(60), range(1000.0f)
{

    channelHeightFloat = (float) channelHeight;

    channelFont = Font("Default", channelHeight*0.6, Font::plain);

    lineColour = Colour(255,255,255);

}

LfpChannelDisplay::~LfpChannelDisplay()
{

}

void LfpChannelDisplay::paint(Graphics& g)
{

    //g.fillAll(Colours::grey);

    g.setColour(Colours::yellow);   // draw most recent drawn sample position
    g.drawLine(canvas->screenBufferIndex+1, 0, canvas->screenBufferIndex+1, getHeight()-channelOverlap);

    //g.setColour(Colours::red); // draw oldest drawn sample position 
    //g.drawLine(canvas->lastScreenBufferIndex, 0, canvas->lastScreenBufferIndex, getHeight()-channelOverlap);

    int center = getHeight()/2;

    if (isSelected)
    {

        g.setColour(Colours::lightgrey);
        g.fillRect(0,center-channelHeight/2,10,channelHeight);
        g.drawLine(0,center+channelHeight/2,getWidth(),center+channelHeight/2);
        g.drawLine(0,center-channelHeight/2,getWidth(),center-channelHeight/2);

        g.setColour(Colour(25,25,25));
        g.drawLine(0,center+channelHeight/4,10,center+channelHeight/4);
        g.drawLine(0,center-channelHeight/4,10,center-channelHeight/4);

    }


    g.setColour(Colour(40,40,40));
    g.drawLine(0, getHeight()/2, getWidth(), getHeight()/2);

    int stepSize = 1;
	int from = 0; // for vertical line drawing in the LFP data 
	int to = 0;
    g.setColour(lineColour);

    //for (int i = 0; i < getWidth()-stepSize; i += stepSize) // redraw entire display
    int ifrom=canvas->lastScreenBufferIndex-3; // need to start drawing a bit before the actual redraw windowfor the interpolated line to join correctly
    if (ifrom<0) ifrom=0;
    int ito = canvas->screenBufferIndex-1;

    if (fullredraw){
        ifrom=canvas->leftmargin;
        ito=getWidth()-stepSize;
        fullredraw=false;
    }

    for (int i = ifrom; i < ito ; i += stepSize) // redraw only changed portion
    {

	   // drawLine makes for ok anti-aliased plots, but is pretty slow
        g.drawLine(i,
                  (canvas->getYCoord(chan, i)/range*channelHeightFloat)+getHeight()/2,
                   i+stepSize,
                    (canvas->getYCoord(chan, i+stepSize)/range*channelHeightFloat)+getHeight()/2);

        if (false) // switched back to line drawing now that we only draw partial updates
        {

    		// // pixel wise line plot has no anti-aliasing, but runs much faster
    		double a = (canvas->getYCoord(chan, i)/range*channelHeightFloat)+getHeight()/2;
    		double b = (canvas->getYCoord(chan, i+stepSize)/range*channelHeightFloat)+getHeight()/2;

    		if (a<b){
    			 from = (a);
    			 to = (b);
    		} else {
    			 from = (b);
    			 to = (a);
    		}
    		
    		if ((to-from) < 40){ // if there is too much vertical range in one pixel, don't draw the full line for speed reasons 
    			for (int j = from; j <= to; j += 1)
    			{
    				g.setPixel(i,j);
    			}
    		} else if ((to-from) < 100){
    			for (int j = from; j <= to; j += 2)
    			{
    				g.setPixel(i,j);
    			}
    		} else {
    			g.setPixel(i,to);
    			g.setPixel(i,from);
    		}

        }
		
    }

 // g.setColour(lineColour.withAlpha(0.7f)); // alpha on seems to decrease draw speed
    g.setFont(channelFont);
    g.setFont(channelHeightFloat*0.6);

    g.drawText(String(chan+1), 10, center-channelHeight/2, 200, channelHeight, Justification::left, false);


}


void LfpChannelDisplay::setRange(float r)
{
    range = r;

    //std::cout << "Range: " << r << std::endl;
}

void LfpChannelDisplay::select()
{
    isSelected = true;
}

void LfpChannelDisplay::deselect()
{
    isSelected = false;
}

void LfpChannelDisplay::setColour(Colour c)
{
    lineColour = c;
}


void LfpChannelDisplay::setChannelHeight(int c)
{
    channelHeight = c;
    channelHeightFloat = (float) channelHeight;
    channelOverlap = channelHeight / 2;
}

int LfpChannelDisplay::getChannelHeight()
{

    return channelHeight;
}

void LfpChannelDisplay::setChannelOverlap(int overlap)
{
    channelOverlap = overlap;
}


int LfpChannelDisplay::getChannelOverlap()
{
    return channelOverlap;
}