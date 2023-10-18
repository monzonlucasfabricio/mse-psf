import numpy as np
import matplotlib.pyplot as plt

# Señal en el dominio de la frecuencia (DFT)
dft_signal = np.array([0, 0, 1, 0, 0])  # Ejemplo de una señal en el dominio de la frecuencia

# Calcular la IDFT
idft_result = np.fft.ifft(dft_signal)

# Crear un arreglo de tiempo para la señal en el dominio del tiempo
t = np.arange(0, len(idft_result))

# Plotear la parte real en el eje X y la parte imaginaria en el eje Y
plt.figure(figsize=(12, 6))
plt.subplot(2, 1, 1)
plt.plot(t, idft_result.real)
plt.title('Parte Real de la Señal (IDFT)')
plt.xlabel('Tiempo (índice)')
plt.ylabel('Amplitud')

plt.subplot(2, 1, 2)
plt.plot(t, idft_result.imag)
plt.title('Parte Imaginaria de la Señal (IDFT)')
plt.xlabel('Tiempo (índice)')
plt.ylabel('Amplitud')

plt.tight_layout()
plt.show()