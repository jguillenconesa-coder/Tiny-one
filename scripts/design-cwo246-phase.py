#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Authored quadrature model, not firmware decryption. Optional numpy/scipy needed.
Four legible float32 coefficients are locked; four protected positions are fitted
for a small quadrature error over 30..18000 Hz at 44.1 kHz. This does not establish
physical OP-1 phase response or original missing coefficients.
"""
import argparse,json,struct
import numpy as np
from scipy.optimize import least_squares
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',required=True);a=p.parse_args()
f=np.geomspace(30,18000,600);w=2*np.pi*f/44100;z=np.exp(-2j*w)
known=[.664528489112854,.13199974596500397,.9952077269554138,.9603016972541809]
def response(x):
 h=np.exp(-1j*w)
 for aa,bb in zip([x[0],x[1],known[0],known[1]],[known[2],known[3],x[2],x[3]]):h*=((bb-z)/(1-bb*z))/((aa-z)/(1-aa*z))
 return h
def cost(x):return response(x).real
r=least_squares(cost,[.985,.86,.84,.4],bounds=([.9604,.6646,.6646,.1321],[.9951,.9602,.9602,.6644]),ftol=1e-13,xtol=1e-13,gtol=1e-13,max_nfev=1000)
def f32(x):return struct.unpack('<f',struct.pack('<f',float(x)))[0]
x=[f32(v) for v in r.x];coef=[x[0],x[1],known[0],known[1],known[2],known[3],x[2],x[3]]
d={'model':'Authored least-squares quadrature design, four protected coefficients substituted','rate':44100,'fitted_positions':[0,1,6,7],'locked_legible_positions':[2,3,4,5],'float32_coefficients':coef,'float32_words':[f'{struct.unpack("<I",struct.pack("<f",v))[0]:08x}' for v in coef],'design_band_hz':[30,18000],'design_grid_points':600,'maximum_design_grid_quadrature_error_degrees':float(np.arcsin(np.max(np.abs(cost(x))))*180/np.pi),'rms_design_grid_cosine_error':float(np.sqrt(np.mean(cost(x)**2))),'limitations':['Four substituted coefficients are not recovered firmware data.','Metric is analytic unsaturated response on the design grid, not hardware/acoustic validation.','No active CWO callback, host integration or physical sample-rate calibration is established.']}
with open(a.output,'w') as out:json.dump(d,out,indent=2);out.write('\n')
print('Authored model saved; design-grid maximum quadrature error:',d['maximum_design_grid_quadrature_error_degrees'])
