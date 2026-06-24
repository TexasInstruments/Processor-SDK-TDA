import tidlruntime
import numpy as np
import sys
current_dir = os.path.dirname(os.path.abspath(__file__))
if current_dir not in sys.path:
    sys.path.append(current_dir)
from utils import populate_config

def get_tidl_performance(session):

    perf = session.get_performance()

    proc_time = (perf["ts:proc_end"] - perf["ts:proc_start"])
    cp_in_time = (perf["ts:copy_in_end"] - perf["ts:copy_in_start"])
    cp_out_time = (perf["ts:copy_out_end"] - perf["ts:copy_out_start"])

    total_time = (perf['ts:run_end'] - perf['ts:run_start'])
    read_total = (perf['ddr:read_end'] - perf['ddr:read_start'])
    write_total = (perf['ddr:write_end'] - perf['ddr:write_start'])

    total_cp_time = cp_in_time + cp_out_time

    # change units
    total_time = total_time/1000000         # Conveting to miliseconds
    total_cp_time = total_cp_time/1000000   # Conveting to miliseconds
    proc_time = proc_time/1000000           # Conveting to miliseconds
    read_total = read_total/1000000         # Conveting to MB/s
    write_total = write_total/1000000       # Conveting to MB/s

    stats = {
        'total_time': total_time,
        'copy_time': total_cp_time,
        'graph_proc_time': proc_time,
        'read_total': read_total,
        'write_total': write_total
    }

    return stats

config = {}
if len(sys.argv) > 1:
    populate_config(config, sys.argv[1:])

session = tidlruntime.InferenceSession(config)
session.dump_options()

input_dict = {}
output_dict = {}
input_details = session.get_input_details()
output_details = session.get_output_details()
for input_detail in input_details:

    np.random.seed(0)

    name = input_detail.name
    shape = input_detail.shape
    type = input_detail.type
    pad = input_detail.pad

    # Create numpy array with random input of specific shape and type
    if (type == tidlruntime.TIDL_SinglePrecFloat):
        input_data = np.random.randn(*shape).astype(np.float32)
    elif (type == tidlruntime.TIDL_UnsignedChar):
        input_data = np.random.randn(*shape).astype(np.uint8)
    elif (type == tidlruntime.TIDL_SignedChar):
        input_data = np.random.randn(*shape).astype(np.int8)
    elif (type == tidlruntime.TIDL_UnsignedShort):
        input_data = np.random.randn(*shape).astype(np.uint16)
    elif (type == tidlruntime.TIDL_SignedShort):
        input_data = np.random.randn(*shape).astype(np.int16)
    elif (type == tidlruntime.TIDL_UnsignedWord):
        input_data = np.random.randn(*shape).astype(np.uint32)
    elif (type == tidlruntime.TIDL_SignedWord):
        input_data = np.random.randn(*shape).astype(np.int32)
    elif (type == tidlruntime.TIDL_UnsignedDoubleWord):
        input_data = np.random.randn(*shape).astype(np.uint64)
    elif (type == tidlruntime.TIDL_SignedDoubleWord):
        input_data = np.random.randn(*shape).astype(np.int62)
    elif (type == tidlruntime.TIDL_Bool):
        input_data = np.random.randn(*shape).astype(np.bool_)
    else:
        print("[ERROR] Invalid datatype")
        sys.exit(-1)

    # Pad the array according to input pads
    padch, padt, padb, padl, padr = pad
    pad_values = [(0, 0),(0, 0),(0, 0),(0, padch),(padt, padb),(padl, padr)]
    print(pad_values)
    input_data = np.pad(input_data, pad_values, 'constant', constant_values=(0))


    input_dict[name] = input_data

# Run the session
outputs = session.run(input_dict)

# Remove output pads
for output_name, output in outputs.items():
    for output_detail in output_details:

        name = output_detail.name
        shape = output_detail.shape
        type = output_detail.type
        pad = output_detail.pad

        if (output_name == name):
            batch, dim1, dim2, channel, height, width = shape
            padch, padt, padb, padl, padr = pad
            outputs[name] = output[:, :, :, 0:channel, padt:(padt+height), padl:(padl+width)]

for name, output in outputs.items():
    #print(output)
    print(output.shape)

print(get_tidl_performance(session))
