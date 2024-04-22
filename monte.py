import ctypes

class Config(ctypes.Structure):
    _fields_ = [
        ("asset_corr", ctypes.c_float),
        ("cut_off", ctypes.c_float),
        ("EAD", ctypes.c_float),
        ("LGD", ctypes.c_float),
        ("pf_size", ctypes.c_int),
        ("n_sims", ctypes.c_int),
    ]

class Result(ctypes.Structure):
    _fields_ = [
        ("L_PF", ctypes.POINTER(ctypes.c_float))
    ]

libmonte = ctypes.CDLL("./monte.so")
libmonte.simulate.argtypes = [Config, Result]
libmonte.simulate.restype = Result

result = Result((ctypes.c_float*1000)())
config = Config(0.05, -2.0, 1.0, 0.6, 100, 1000)
result = libmonte.simulate(config, result)

print(result)
for i in range(10):
    print(result.L_PF[i])
