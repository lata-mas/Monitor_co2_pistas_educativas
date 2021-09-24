import pandas as pd
import numpy as np
from dateutil.parser import parse


def fecha_geo(file,renglon=3):
    fp = open(file,encoding="iso-8859-1")
    for i, line in enumerate(fp):
        if i == renglon:
            fecha = line
            break
    fp.close()
    fecha = parse(fecha[6:])
    return fecha

def importa_geo(file,fecha,renglon=128):
    geo = pd.read_csv(file,delimiter="\t",skiprows=renglon,encoding="iso-8859-1")
    geo.sec = pd.to_timedelta(geo.sec,unit='S')
#     geo['fecha'] = fecha - geo.sec
    geo['fecha'] = fecha + geo.sec - np.max(geo.sec)

    geo.set_index("fecha",inplace=True)
    return geo

def importa_fluke(archivo):
    tmp = pd.read_csv(archivo,usecols=[7,11],names=["fluke","tiempo"],skiprows=1,encoding='iso-8859-1')
    tmp.tiempo = pd.to_datetime(tmp.tiempo,dayfirst=True)
    tmp.set_index("tiempo",inplace=True)
    tmp.fluke = tmp.fluke.str.replace(" ppm","")
    tmp.fluke = tmp.fluke.astype("float64")
    return tmp
