/*
 *
 * Copyright (c) 2019-2026 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <filesystem>
#include <limits.h>

#include <graphics/include/Mesh3DRenderer.h>
#include <graphics/include/ModelFactory.h>
#include <utils/include/logger.h>
#include <utils/include/resources.h>
#include <car.h>

#define degreesToRadians(x) x *(3.141592f / 180.0f)

extern glm::mat4 mProjection[];
extern glm::mat4 mView[];
extern glm::mat4 mMVP_car[];
extern glm::vec3 vCameraDir[];
extern int       num_viewports;

using namespace ti::graphics;
using std::string;

using Mesh3DRendererPtr = std::shared_ptr<Mesh3DRenderer>;
using ModelPtr          = std::shared_ptr<Model>;

struct CarData
{
        const char           *foldername{ nullptr };
        const char           *filename{ nullptr };
        ModelPtr              model{ nullptr };
        std::vector<uint32_t> meshIds;
        GLint                 xrot_degrees{ 0 };
        GLint                 yrot_degrees{ 0 };
        GLint                 zrot_degrees{ 0 };
        GLfloat               scale{ 1.0f };
};

static CarData carData[] = { {
    foldername: "/CarConcept/",
    filename: "CarConcept.gltf",
    model: nullptr,
    meshIds: {},
    xrot_degrees: 90,
    yrot_degrees: 180,
    zrot_degrees: 0,
    scale: 25.0f,
} };

static Mesh3DRendererPtr meshRenderer{ nullptr };
static int32_t           actvCarIndex = 0;
static int32_t           numCarModels = sizeof(carData) / sizeof(CarData);

// Configure lighting for SRV coordinate system (cameras at Z=150-640)
// Scale lights from graphics_engine config to match SRV's coordinate system (~50x scale)
// Keep bottom light as it provides essential back-side illumination
std::vector<glm::vec3> lightPositions = {
    glm::vec3(250.0f, 250.0f, 250.0f),  // Main light from front-top-right
    glm::vec3(-250.0f, 250.0f, 250.0f), // Fill light from front-top-left
    glm::vec3(0.0f, -150.0f, 100.0f), // Bottom/rear light for back-side illumination
    glm::vec3(0.0f, 150.0f, -400.0f), // Back light
    glm::vec3(400.0f, 100.0f, 0.0f),  // Right side light
    glm::vec3(-400.0f, 100.0f, 0.0f)  // Left side light
};

// Use balanced light colors with moderate brightness
std::vector<glm::vec3> lightColors = {
    glm::vec3(1.0f, 0.95f, 0.9f), // Warm main light
    glm::vec3(0.9f, 0.9f, 1.0f),  // Cool fill light
    glm::vec3(0.8f, 0.8f, 0.9f),  // Bottom/rear light
    glm::vec3(0.7f, 0.7f, 0.8f),  // Back light
    glm::vec3(0.9f, 0.8f, 0.7f),  // Right side light
    glm::vec3(0.7f, 0.8f, 0.9f)   // Left side light
};

// Reduce intensity for all lights except bottom light to maintain back-side illumination
std::vector<float> lightIntensities = { 1.0f, 0.6f, 1.0f, 0.4f, 0.5f, 0.5f };

int car_init(int32_t screen_width, int32_t screen_height)
{
    int32_t status = 0;
    int32_t numCarsSetup = 0;

    // Create renderer
    meshRenderer = std::make_shared<Mesh3DRenderer>(screen_width, screen_height);

    if (!meshRenderer->initialize())
    {
        LOG_ERROR("Failed to initialize renderer");
        status = -1;
    }
    else
    {
        meshRenderer->setLights(lightPositions, lightColors, lightIntensities);
    }

    if (status == 0)
    {
        for (int i = 0; i < numCarModels; i++)
        {
            const auto &rsrcPath = ti::utils::getResourcePath();
            const auto &modelPath = rsrcPath + "/models" +
                                    carData[i].foldername +
                                    carData[i].filename;

            if (!std::filesystem::exists(modelPath))
            {
                LOG_DEBUG("Model [%s] does not exist.\n", modelPath.c_str());
                continue;
            }

            carData[i].model = ModelFactory::loadFromFile(modelPath);

            if (!carData[i].model)
            {
                LOG_ERROR("Failed to load model: %s", modelPath);
                status = -1;
            }
            else
            {
                // Render using the model's implementation
                carData[i].meshIds = carData[i].model->render(*meshRenderer);
            }

            numCarsSetup++;
        }

        if (!numCarsSetup)
        {
            status = -1;
        }
    }

    return status;
}

void car_draw(int viewport_id)
{
    // Clear the color and depth buffer
    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    auto &actvCar = carData[actvCarIndex];

    for (uint32_t meshId : actvCar.meshIds)
    {
        float xAngle = degreesToRadians(actvCar.xrot_degrees);
        float yAngle = degreesToRadians(actvCar.yrot_degrees);
        float zAngle = degreesToRadians(actvCar.zrot_degrees);

        auto pos   = glm::vec3(0.0f);
        auto rot   = glm::vec3(xAngle, yAngle, zAngle);
        auto scale = glm::vec3(actvCar.scale);

        meshRenderer->updateTransform(meshId, pos, rot, scale);
    }

    glm::mat4 viewProj = mProjection[viewport_id] * mView[viewport_id];
    meshRenderer->render(viewProj);
}

int ReleaseView()
{
    return 0;
}

void car_updateView(int i)
{
}

void car_change()
{
    do
    {
        actvCarIndex = (actvCarIndex + 1) % numCarModels;
    } while (carData[actvCarIndex].model == nullptr);

    for (int i = 0; i < num_viewports; i++)
    {
        car_updateView(i);
    }
}
