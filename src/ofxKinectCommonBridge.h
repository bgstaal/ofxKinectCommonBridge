#pragma once

//#define KCB_ENABLE_FT
//#define KCB_ENABLE_SPEECH

#include "KinectCommonBridgeLib.h"
#include "NuiSensor.h"
#include "ofMain.h" // this MUST come after KCB!!! Not sure you need NuiSensor.h if using KCB

#pragma comment (lib, "KinectCommonBridge.lib") // add path to lib additional dependency dir $(TargetDir)
#pragma comment(lib, "FaceTrackLib.lib")

class ofxKCBFace  {

public:

	enum FACE_POSITIONS {

	};

	enum FEATURE {
		LEFT_EYE, RIGHT_EYE, MOUTH, NOSE, CHIN, LEFT_EAR, RIGHT_EAR
	};

	ofxKCBFace & operator=(const ofxKCBFace & rhs) {
		rotation = rhs.rotation;
		translation = rhs.translation;
		//mesh = rhs.mesh; // this is causing problems
		rect = rhs.rect;

		return *this;
	};

	ofVec3f rotation, translation;
	ofRectangle rect;
	ofMesh mesh;
	
	ofVec3f getLocationByIdentifier(FACE_POSITIONS position);
	ofRectangle getFeatureBounding(FACE_POSITIONS position);

};

// if you want to use events, subscribe to this, otherwise
class ofxKCBSpeechEvent : public ofEventArgs
{

public:

	std::string detectedSpeech;
	int confidence;

	static ofEvent<ofxKCBSpeechEvent> event;

};

// poll for this very simple data object for speech data
class SpeechData {
public:
	std::string detectedSpeech;
	int confidence;
};

class SkeletonBone
{
public:
	enum TrackingState {
		NotTracked,
		Tracked,
		Inferred
	};
	// lots of constness because we're putting these in a map and that
	// copies stuff all over the place
	const ofQuaternion getCameraRotation();
	const ofMatrix4x4 getCameraRotationMatrix();

	const ofVec3f& getStartPosition();
	const ofVec3f getScreenPosition();
	const ofQuaternion&	getRotation();
	const ofMatrix4x4& getRotationMatrix();

	const int getStartJoint();
	int getEndJoint();

	TrackingState getTrackingState();

	SkeletonBone( const Vector4& inPosition, const _NUI_SKELETON_BONE_ORIENTATION& bone, const NUI_SKELETON_POSITION_TRACKING_STATE& trackingState );

private:

	ofMatrix4x4 cameraRotation;
	int	endJoint;
	int	startJoint;
	ofVec3f	position;
	ofMatrix4x4	rotation;
	ofVec2f screenPosition;
	TrackingState trackingState;
};

typedef map<_NUI_SKELETON_POSITION_INDEX, SkeletonBone> Skeleton;

class ofxKinectCommonBridge : protected ofThread {
  public:
	ofxKinectCommonBridge();

	// new API
	bool initSensor( int id = 0 );
	bool initDepthStream( int width, int height, bool nearMode = false, bool mapColorToDepth = false );
	bool initColorStream( int width, int height, bool mapColorToDepth = false );
	bool initFaceTracking(); // no params, can't use with other stuff either.
	bool initIRStream( int width, int height );
	bool initSkeletonStream( bool seated );
	bool initPointCloud( unsigned int nthPixelAsPoint = 2, float scale = 100.0f, bool useColor = true );
	bool initPointCloud( unsigned int nthPixelAsPoint, ofMatrix4x4 transform, bool useColor = true );
	
	bool start();

	// audio functionality
	bool startAudioStream();
	bool initSpeech();
	bool loadGrammar(string filename);

	// speech
	bool hasNewSpeechData();
	SpeechData getNewSpeechData();

	void stop();

  	/// is the current frame new?
	bool isFrameNew();
	bool isFrameNewVideo();
	bool isFrameNewDepth();
	bool isNewSkeleton();
	bool isFaceNew() {
		return bIsFaceNew;
	}

	void setDepthClipping(float nearClip=500, float farClip=5000);
	void setPointCloudTransform(const ofMatrix4x4 &t);
	ofMatrix4x4 getPointCloudTransform();
	
	/// updates the pixel buffers and textures
	///
	/// make sure to call this to update to the latest incoming frames
	void update();
	ofPixels& getColorPixelsRef();
	ofPixels & getDepthPixelsRef();       	///< grayscale values
	ofShortPixels & getRawDepthPixelsRef();	///< raw 11 bit values

	/// enable/disable frame loading into textures on update()
	void setUseTexture(bool bUse);
	// you can just not use any streams if you want
	void setUseStreams(bool bUse);

	/// draw the video texture
	void draw(float x, float y, float w, float h);
	void draw(float x, float y);
	void draw(const ofPoint& point);
	void draw(const ofRectangle& rect);

	/// draw the grayscale depth texture
	void drawDepth(float x, float y, float w, float h);
	void drawDepth(float x, float y);
	void drawDepth(const ofPoint& point);
	void drawDepth(const ofRectangle& rect);

	void drawIR( float x, float y, float w, float h );

	ofVboMesh &getPointCloud();

	vector<Skeleton> &getSkeletons();
	void drawSkeleton(int index);

	ofTexture &getDepthTexture() {
		return depthTex;
	}

	ofTexture &getColorTexture() {
		return videoTex;
	}

	// face tracking
	ofTexture &getFaceTrackingTexture()
	{
		return faceTrackingTexture;
	}

	ofxKCBFace& getFaceData();

	wstring getPortID()
	{
		return portID;
	}

	string getPortIDAsString()
	{
		string id(portID.begin(), portID.end());
		return id;
	}

	void setSpeechGrammarFile(string path) {
		grammarFile = path;
	}
	bool initAudio();

  private:

    KCBHANDLE hKinect;
	KINECT_IMAGE_FRAME_FORMAT depthFormat;
	KINECT_IMAGE_FRAME_FORMAT colorFormat;
	NUI_SKELETON_FRAME k4wSkeletons;

  	bool bInited;
	bool bStarted;
	vector<Skeleton> skeletons;
	ofVboMesh pointCloud;
	ofMesh pointCloudBack;
	ofMatrix4x4 pointCloudTransform;
	unsigned int pointCloudNthPixelAsPoint;
	bool bPointCloudUseColor;
	wstring portID;

	void updateDepthPixels();
	void updateIRPixels();
	bool bNearWhite;
	float nearClipping, farClipping;
	unsigned char nearColor, farColor;

	void updateFaceTrackingData( IFTResult* ftResult );

  	bool bUseTexture;
	ofTexture depthTex; ///< the depth texture
	ofTexture videoTex; ///< the RGB texture

	// face
	ofTexture faceTrackingTexture;
	//ofTexture irTex;

	ofPixels videoPixels;
	ofPixels videoPixelsBack;			///< rgb back
	ofPixels depthPixels;
	unique_ptr<NUI_DEPTH_IMAGE_PIXEL[]> depthImagePixels; // depth pixels with full range
	unique_ptr<NUI_DEPTH_IMAGE_PIXEL[]> depthImagePixelsBack;

	ofShortPixels irPixelsRaw;
	ofShortPixels irPixelsBackRaw;
	ofPixels irPixels;
	ofPixels irPixelsBack;

	bool bIsFrameNewVideo;
	bool bNeedsUpdateVideo;
	bool bIsFrameNewDepth;
	bool bNeedsUpdateDepth;
	bool bNeedsUpdateSkeleton;
	bool bIsSkeletonFrameNew;
	bool bUpdateSpeech;
	bool bUpdateFaces;
	bool bIsFaceNew;
	bool bUseStreams;

	bool bProgrammableRenderer;

	// speech
	bool bHasLoadedGrammar;
	bool bUsingSpeech;
	string grammarFile;
	SpeechData speechData;
	float speechConfidenceThreshold; // make a way to set this

	// audio
	bool bUsingAudio;

	// face
	bool bIsTrackingFace;
	// double buffer for faces
	ofxKCBFace faceDataBack, faceData; // only single face at the moment
	// camera params for face tracking
	FT_CAMERA_CONFIG depthCameraConfig;
	FT_CAMERA_CONFIG videoCameraConfig;
    FLOAT pSUCoef;
    FLOAT zoomFactor;


	bool bVideoIsInfrared;
	bool bUsingSkeletons;
	bool bUsingDepth;
	bool bUsingPointCloud;

	BYTE *irPixelByteArray;

	void threadedFunction();

	bool mappingColorToDepth;
	bool mappingDepthToColor;
	bool beginMappingColorToDepth, beginMappingDepthToColor;

	NUI_IMAGE_RESOLUTION colorRes;
	NUI_IMAGE_RESOLUTION depthRes;
};
