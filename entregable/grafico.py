import matplotlib.pyplot as plt
import numpy as np

datos = np.loadtxt("tiempos.csv", delimiter=',')
datos2 = np.loadtxt("tiempos2.csv", delimiter=',')
datos3 = np.loadtxt("tiempos3.csv", delimiter=',')

plt.plot(datos[:,0], datos[:,1], 'o', label="maximum1")
plt.plot(datos[:,0], datos2[:,1], 's', label="maximum2")
plt.plot(datos[:,0], datos3[:,1], '^', label="maximum3")

plt.ylim(ymin=0, ymax=27)
plt.xlim(xmin=0, xmax=11)
plt.ylabel("Tiempo de ejecución (ms)")
plt.xlabel("Cantidad de threads")
# plt.legend()
plt.tight_layout()
plt.show()