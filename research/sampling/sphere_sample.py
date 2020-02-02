import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

def sphere_samples(e0, e1):
    cos_the = 2*e1 - 1
    phi = e0 * 2 * np.pi
    sin_the = np.sqrt(1 - cos_the*cos_the)
    z = cos_the 
    x = sin_the*np.cos(phi)
    y = sin_the*np.sin(phi)
    return x, y, z

def cos_sphere_samples(e0, e1):
    z = np.zeros(e1.shape)
    z[e1 < 0.5] = np.sqrt(2*e1[e1 < 0.5]) 
    z[e1 >= 0.5] = -np.sqrt(2*(1-e1[e1 >= 0.5]))
    r = np.sqrt(1.0 - z*z)

    phi = e0 * 2 * np.pi
    x = r*np.cos(phi)
    y = r*np.sin(phi)
    return x, y, z

n = 500
e0 = np.random.uniform(low=0, high=1, size=n)
e1 = np.random.uniform(low=0, high=1, size=n)
x, y, z = sphere_samples(e0, e1)
# x, y, z = cos_sphere_samples(e0, e1)

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

ax.scatter(x, y, z, c='r', marker='o')

ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')

plt.show()
