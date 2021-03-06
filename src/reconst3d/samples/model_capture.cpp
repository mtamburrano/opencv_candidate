#include <iostream>

#include <opencv_candidate_reconst3d/reconst3d.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

static Mat defaultCameraMatrix()
{
    float vals[] = {525., 0., 3.1950000000000000e+02,
                    0., 525., 2.3950000000000000e+02,
                    0., 0., 1.};
    return Mat(3,3,CV_32FC1,vals).clone();
}

static void releaseUnusedFrames(const Ptr<TrajectoryFrames>& keyframesData, vector<Mat>& images, vector<Mat>& depthes)
{
    for(size_t imageIndex = 0; imageIndex < images.size(); imageIndex++)
    {
        bool isKeyframe = false;
        for(size_t keyframeIndex = 0; keyframeIndex < keyframesData->frames.size(); keyframeIndex++)
        {
            if(keyframesData->frames[keyframeIndex]->ID == static_cast<int>(imageIndex))
            {
                isKeyframe = true;
                break;
            }
        }
        if(!isKeyframe)
        {
            images[imageIndex].release();
            depthes[imageIndex].release();
        }
    }
}

int main(int argc, char** argv)
{
    if(argc != 3  && argc != 4)
    {
        cout << "Format: " << argv[0] << " train_dirname construct_if_loop_closure_only [model_filename]" << endl;
        cout << "   train_dirname - a path to the directory with TOD-like training base." << endl;
        cout << "   construct_if_loop_closure_only - 0 or 1. If 1 the model reconstruction will be run if the loop closure was detected. If 0,"
                " it will be run in any case (and result can be not very accurate in this case)." << endl;
        cout << "   model_filename - an optional parameter, it's a filename that will be used to save trained model." << endl;
        return -1;
    }

    // Load the data
    const string dirname = argv[1];
    bool constructIfLoopClosureOnly = true;

    if(string(argv[2]) == "0")
        constructIfLoopClosureOnly = false;
    else if(string(argv[2]) != "1")
    {
        cout << "Incorrect second parameter passed to the application." << endl;
        return -1;
    }

    vector<Mat> bgrImages, depthes;
    loadTODLikeBase(dirname, bgrImages, depthes);
    if(bgrImages.empty())
    {
        cout << "Can not load the data from given directory of the base: " << dirname << endl;
        return -1;
    }

    Mat cameraMatrix = defaultCameraMatrix();

    OnlineCaptureServer onlineCaptureServer;
    onlineCaptureServer.set("cameraMatrix", cameraMatrix);
    CV_Assert(!bgrImages[0].empty());
    onlineCaptureServer.initialize(bgrImages[0].size());//, TrajectoryFrames::VALIDFRAME);
    for(size_t i = 0; i < bgrImages.size(); i++)
        onlineCaptureServer.push(bgrImages[i], depthes[i], i);

    Ptr<TrajectoryFrames> trajectoryFrames = onlineCaptureServer.finalize();
    if(constructIfLoopClosureOnly && !onlineCaptureServer.get<bool>("isLoopClosed"))
        return -1;

#if 1
    releaseUnusedFrames(trajectoryFrames, bgrImages, depthes);
#endif

    ModelReconstructor reconstructor;
    reconstructor.set("isShowStepResults", false);

    Ptr<ObjectModel> model;
    reconstructor.reconstruct(trajectoryFrames, cameraMatrix, model);

    if(argc == 4)
        model->write_ply(argv[3]);

    model->show();

    return 0;
}
