
# Draw charts from simulations
import matplotlib;
import numpy;
import matplotlib.pyplot as plt;
import matplotlib.cbook as cbook;

CMF_PATH = "visual/data/cmf"

cmf_data = numpy.genfromtxt(CMF_PATH, delimiter = ',', skip_header = 1,
        name = ['time', 'arrival', 'complete', 'finish', 'finish(c)']);

fig = plt.figure()

cmf = fig.add_subplot(111)

cmf.set_title("Progress by time");
cmf.set_xlabel("Time (s)")
cmf.set_ylabel("Percentage (%)")

cmf.plot(cmf_data['time'], cmf_data['arrival'], color = 'r', label = "Arrival")
cmf.plot(cmf_data['time'], cmf_data['arrival'], color = 'b', label = "Complete")
cmf.plot(cmf_data['time'], cmf_data['arrival'], color = 'g', label = "Finish")
cmf.plot(cmf_data['time'], cmf_data['arrival'], color = 'o',
        label = "Finish(c)")

plt.savefig()



