from seispy.synthetics import plane
from seispy.plotting import image, wiggle
import matplotlib.pyplot as plt

wiggle(plane())
plt.show()

image(plane())
plt.show()