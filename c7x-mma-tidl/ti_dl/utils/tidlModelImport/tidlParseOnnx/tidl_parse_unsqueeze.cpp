
#include "tidl_parse_onnx.h"
using namespace std;
using namespace onnx;
 
template<> int32_t TidlParseOnnx:: parse<OnnxStr("Unsqueeze")> ()
{
    layer.layerType = TIDL_UnsqueezeLayer;
    layer.numInBufs = 1;
    NodeProto node  = graph.node(index);
      int32_t axes_status = -1, axesIdx = -1;
    int32_t j, ii, numDim = 0;
    int32_t num_axes_4_squeeze = -1;
    int32_t axes[TIDL_DIM_MAX];

    sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

    axesIdx = getAttrIdx(node, "axes");
    if (axesIdx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
        axes_status = axesIdx;
        num_axes_4_squeeze = node.attribute(axesIdx).ints_size();
        for(ii = 0; ii< num_axes_4_squeeze; ii++)
        {
        getIntAttr(node, "axes",   &axes[ii], ii);
        }
    }
    else if(md.numVarInputs <= 1)
    {
        /* axes can be an input in opset 18*/
        sBuffer_t buf;
        axes_status = copyFloatConst(graph, index, 1, buf, INPUT_REQUIRED);
        if (axes_status != TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
        {
            int32_t status = 0;
            TensorProto tensor = getInitializerTensor(graph, node.input(1), index, status);
            num_axes_4_squeeze = buf.bufSize;
            for (int i = 0; i < num_axes_4_squeeze; i++)
            {
                if (tensor.data_type() == TensorProto_DataType_INT64)
                {
                    axes[i] = (int32_t)(*((int64_t*)buf.ptr + i));
                }
                else if (tensor.data_type() == TensorProto_DataType_INT32)
                {
                    axes[i] = (int32_t)(*((int32_t*)buf.ptr + i));
                }
                else if (tensor.data_type() == TensorProto_DataType_INT8)
                {
                    axes[i] = (int32_t)(*((int8_t*)buf.ptr + i));
                }
                else if (tensor.data_type() == TensorProto_DataType_UINT8)
                {
                    axes[i] = (int32_t)(*((uint8_t*)buf.ptr + i));
                }
            }
        }
        free (buf.ptr);
    }
    else
    {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Squeeze layer : Variable input for axes is not supported");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    if (axes_status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Squeeze layer : No axis given for Squeeze");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    /** Get numDim from data tensor (input 0), whether it is variable or const */
    numDim = 4;
    for (int32_t k = 0; k < (int32_t)layer.allowlistingMetaData.varTensorIndices.size(); k++)
    {
      if (layer.allowlistingMetaData.varTensorIndices[k] == 0)
      {
        if (layer.allowlistingMetaData.varTensorsDims[k].size() != 0)
          numDim = layer.allowlistingMetaData.varTensorsDims[k].size();
        break;
      }
    }
    for (int32_t k = 0; k < (int32_t)layer.allowlistingMetaData.constTensorIndices.size(); k++)
    {
      if (layer.allowlistingMetaData.constTensorIndices[k] == 0)
      {
        if (layer.allowlistingMetaData.constTensorsDims[k].size() != 0)
          numDim = layer.allowlistingMetaData.constTensorsDims[k].size();
        break;
      }
    }

    for(ii = 0; ii< TIDL_DIM_MAX; ii++)
    {
        layer.layerPCParams.unsqueezeParams.axes[ii] = 0;
        layer.layerPCParams.unsqueezeParams.axis[ii] = 0;
    }

    /* Updating the axes to TIDL max dimensions*/
    for (ii = 0; ii < num_axes_4_squeeze; ii++)
    {
        if(axes[ii] >= 0)
        {
          axes[ii] += (TIDL_DIM_MAX - (num_axes_4_squeeze + numDim));
        }
        else
        {
          axes[ii] += TIDL_DIM_MAX;
        }
        layer.layerPCParams.unsqueezeParams.axes[ii] = axes[ii];
    }

    for(ii = 0; ii< TIDL_DIM_MAX; ii++)
    {
        for(j=0; j < num_axes_4_squeeze; j++)
        {
            if(ii == axes[j])
            {
                layer.layerPCParams.unsqueezeParams.axis[ii] = 1; // squeeze this particular axis
            }
        }
    }

    /* If data input (input 0) is a constant initializer, store it in layer.weights
     * so tidl_addConstDataLayers can create the appropriate ConstDataLayer */
    for (int32_t i = 0; i < (int32_t)md.numConstInputs; i++)
    {
        if (md.constTensorIndices[i] == 0)
        {
            int32_t constStatus = copyFloatConst(graph, index, 0, layer.weights, INPUT_REQUIRED);
            if (constStatus != 0)
            {
                TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Unsqueeze layer : Unable to read constant data input");
                return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
            }
            break;
        }
    }

    return 0;
}