import numpy as np, struct, zipfile, math

# =====================================================
# GroundStation Case V18 COMPACT OPEN FLOOR SMOOTH BUTTONS NO REAR LIP
# - Frente reducido: 110 x 65 mm aprox.
# - Profundidad mantenida: 65 mm aprox.
# - Piso interno libre: sin division entre bateria y StepDown.
# - Paredes de 3 mm.
# - Sin labio trasero interno ancho para reducir soportes.
# - Agujeros de botones con mayor resolucion.
# - Pantalla sin hundimiento, pegada detras del frente.
# - Ventana pantalla visible aprox. 54 x 28 mm.
# - Botones mas juntos debajo de la pantalla.
# - Geometria manifold por voxel para Cura.
# =====================================================

RES = 0.4
W = 110.0
D = 65.0
H = 65.0
wall = 3.0
front_th = 3.0

NX = int(round(W / RES))
NY = int(round(D / RES))
NZ = int(round(H / RES))
occ = np.ones((NX, NY, NZ), dtype=bool)

def ix(x): return int(round((x + W/2) / RES))
def iy(y): return int(round(y / RES))
def iz(z): return int(round(z / RES))

def fill_box(x1,x2,y1,y2,z1,z2,val=True):
    xa=max(0,min(NX,ix(x1))); xb=max(0,min(NX,ix(x2)))
    ya=max(0,min(NY,iy(y1))); yb=max(0,min(NY,iy(y2)))
    za=max(0,min(NZ,iz(z1))); zb=max(0,min(NZ,iz(z2)))
    occ[xa:xb, ya:yb, za:zb] = val

# Main hollow cavity, open rear.
fill_box(-W/2+wall, W/2-wall, front_th, D, wall, H-wall, False)

# V18: rear lip removed.
# The rear remains open with only the normal 3 mm body walls.
# The rear cover is intended to be glued with hot silicone, so no internal lip is required.

# Screen opening: visible area is 50 x 25 mm, opening gives tolerance.
fill_box(-27, 27, 0, front_th+1, 31, 59, False)

# TFT PCB support ledges immediately behind the front.
# PCB measured about 62 x 29 mm, so support frame is slightly larger.
ledge_y1 = front_th
ledge_y2 = front_th + 4
fill_box(-34, 34, ledge_y1, ledge_y2, 28, 31, True)  # bottom ledge
fill_box(-34, 34, ledge_y1, ledge_y2, 59, 62, True)  # top ledge
fill_box(-34, -31, ledge_y1, ledge_y2, 28, 62, True) # left ledge
fill_box(31, 34, ledge_y1, ledge_y2, 28, 62, True)   # right ledge

# Button holes, 7 mm threaded body, 7.8 mm hole.
button_radius = 4.05
for cx in [-24.0, 0.0, 24.0]:
    cz = 13.0
    for xi0 in range(max(0,ix(cx-button_radius-1)), min(NX,ix(cx+button_radius+1))):
        xw = xi0 * RES - W/2
        for zi0 in range(max(0,iz(cz-button_radius-1)), min(NZ,iz(cz+button_radius+1))):
            zw = zi0 * RES
            if (xw-cx)**2 + (zw-cz)**2 <= button_radius**2:
                occ[xi0, iy(0):iy(26), zi0] = False

# Right side cutouts: USB-C charge and ON/OFF switch.
# Reduced height for USB-C, switch remains larger.
fill_box(W/2-wall, W/2, 22, 37, 43, 49, False)  # USB-C charge
fill_box(W/2-wall, W/2, 22, 40, 20, 32, False)  # ON/OFF switch

# TP4056 platform: flat, attached to right wall, below USB-C.
fill_box(W/2-wall-38, W/2-wall, 16, 43, 38, 41, True)

# V17: Open internal floor.
# Removed StepDown/Battery divider and raised rails.
# StepDown and battery holder are intended to be glued directly to the 3 mm bottom wall.
# This maximizes real interior space and avoids interference during assembly.

# STL conversion.
dirs=[
    ((-1,0,0), lambda x,y,z:[(x,y,z),(x,y+1,z),(x,y+1,z+1),(x,y,z+1)]),
    ((1,0,0),  lambda x,y,z:[(x+1,y,z),(x+1,y,z+1),(x+1,y+1,z+1),(x+1,y+1,z)]),
    ((0,-1,0), lambda x,y,z:[(x,y,z),(x,y,z+1),(x+1,y,z+1),(x+1,y,z)]),
    ((0,1,0),  lambda x,y,z:[(x,y+1,z),(x+1,y+1,z),(x+1,y+1,z+1),(x,y+1,z+1)]),
    ((0,0,-1), lambda x,y,z:[(x,y,z),(x+1,y,z),(x+1,y+1,z),(x,y+1,z)]),
    ((0,0,1),  lambda x,y,z:[(x,y,z+1),(x,y+1,z+1),(x+1,y+1,z+1),(x+1,y,z+1)]),
]

def normal(p1,p2,p3):
    ax,ay,az=[p2[i]-p1[i] for i in range(3)]
    bx,by,bz=[p3[i]-p1[i] for i in range(3)]
    nx=ay*bz-az*by; ny=az*bx-ax*bz; nz=ax*by-ay*bx
    l=math.sqrt(nx*nx+ny*ny+nz*nz) or 1
    return nx/l, ny/l, nz/l

def write_binary(path, triangles, name, center_x):
    with open(path,'wb') as f:
        header=name.encode('ascii')[:80]
        f.write(header + b' '*(80-len(header)))
        f.write(struct.pack('<I', len(triangles)))
        for tri in triangles:
            n=normal(tri[0],tri[1],tri[2])
            f.write(struct.pack('<3f', *n))
            for p in tri:
                f.write(struct.pack('<3f', p[0]-center_x, p[1], p[2]))
            f.write(struct.pack('<H',0))

tri=[]
xs,ys,zs=np.nonzero(occ)
for x,y,z in zip(xs,ys,zs):
    for (dx,dy,dz),vf in dirs:
        nx=x+dx; ny=y+dy; nz=z+dz
        neighbor=(0 <= nx < NX and 0 <= ny < NY and 0 <= nz < NZ and occ[nx,ny,nz])
        if not neighbor:
            v=[]
            for p in vf(x,y,z):
                v.append((p[0]*RES, p[1]*RES, p[2]*RES))
            tri.append([v[0],v[1],v[2]])
            tri.append([v[0],v[2],v[3]])

write_binary('/mnt/data/GroundStation_Case_Body_v18_SMOOTH_NO_REAR_LIP.stl', tri, 'GroundStation_Case_Body_v18_SMOOTH_NO_REAR_LIP', W/2)

# Rear cover compact.
CW=107
CH=62
CD=6
cover=np.ones((int(CW), int(CD), int(CH)), dtype=bool)
# pry notch right side
cover[int(CW)-7:int(CW), :, 12:24] = False
# ESP32 pad on inner side, sized for 50 x 25 board plus tolerance.
cover[(int(CW)//2)-29:(int(CW)//2)+29, 3:6, 24:56] = True
cover[(int(CW)//2)-32:(int(CW)//2)-29, 3:8, 24:56] = True
cover[(int(CW)//2)+29:(int(CW)//2)+32, 3:8, 24:56] = True
cover[(int(CW)//2)-29:(int(CW)//2)+29, 3:8, 24:27] = True
# friction lips
cover[5:int(CW)-5, 3:6, 5:7] = True
cover[5:int(CW)-5, 3:6, int(CH)-7:int(CH)-5] = True
cover[5:7, 3:6, 5:int(CH)-5] = True
cover[int(CW)-7:int(CW)-5, 3:6, 28:int(CH)-5] = True
cover[int(CW)-7:int(CW)-5, 3:6, 5:10] = True

tri2=[]
xs,ys,zs=np.nonzero(cover)
for x,y,z in zip(xs,ys,zs):
    for (dx,dy,dz),vf in dirs:
        nx=x+dx; ny=y+dy; nz=z+dz
        neighbor=(0 <= nx < CW and 0 <= ny < CD and 0 <= nz < CH and cover[nx,ny,nz])
        if not neighbor:
            v=vf(x,y,z)
            tri2.append([v[0],v[1],v[2]])
            tri2.append([v[0],v[2],v[3]])

with open('/mnt/data/GroundStation_Case_RearCover_v18_SMOOTH_NO_REAR_LIP.stl','wb') as f:
    name=b'GroundStation_Case_RearCover_v18_SMOOTH_NO_REAR_LIP'
    f.write(name + b' '*(80-len(name)))
    f.write(struct.pack('<I',len(tri2)))
    for t in tri2:
        n=normal(t[0],t[1],t[2])
        f.write(struct.pack('<3f',*n))
        for p in t:
            f.write(struct.pack('<3f',p[0]-CW/2,p[1],p[2]))
        f.write(struct.pack('<H',0))

with zipfile.ZipFile('/mnt/data/GroundStation_Case_STL_v18_SMOOTH_NO_REAR_LIP.zip','w') as z:
    z.write('/mnt/data/GroundStation_Case_Body_v18_SMOOTH_NO_REAR_LIP.stl','GroundStation_Case_Body_v18_SMOOTH_NO_REAR_LIP.stl')
    z.write('/mnt/data/GroundStation_Case_RearCover_v18_SMOOTH_NO_REAR_LIP.stl','GroundStation_Case_RearCover_v18_SMOOTH_NO_REAR_LIP.stl')

print('V18 smooth no rear lip generated')
print('body triangles',len(tri))
print('cover triangles',len(tri2))
