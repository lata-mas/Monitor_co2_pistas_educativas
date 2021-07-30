import pandas as pd
import matplotlib.pyplot as plt
from dateutil.parser import parse

def analiza_sensor(df,fluke,sensor="None",intervalo="1S",
                   int_promedio="5Min",grafica=True,imprime=False,
                   zoom=False):
    tmp = pd.DataFrame(df[sensor])
    tmp.dropna(inplace=True)
    tmp["derivada"]     = tmp.diff().rolling(intervalo).mean()
    tmp["tiempo"]       = tmp.index
    tmp["transcurrido"] = tmp.tiempo - tmp.tiempo.iloc[0]
    tmp['transcurrido'] = tmp['transcurrido'].dt.total_seconds()/60
    
    
#     tmp[tmp.derivada<=0].head(1).values
    resultados = obtiene_resultados(tmp,fluke,sensor,int_promedio)
    if grafica:
        grafica_sensor(tmp,fluke,intervalo,zoom)
    if imprime:
        print(sensor,resultados[["TR+","Error"]])
    return resultados


def grafica_sensor(df,fluke,intervalo,zoom):
    fig, ax = plt.subplots(2,figsize=(12,10),sharex=True)
    nombre = df.columns
    ax[0].plot(df[nombre[0]].rolling(intervalo).mean(),"go",label=nombre[0])
    ax[0].plot(fluke.fluke.rolling(intervalo).mean(),"ro",label='fluke')
    
    ax[0].set_ylabel("$CO_2$ [ppm]")
    ax[0].set_title("Figura 1")
    ax[1].set_title("Figura 2")
    ax[1].plot(df.derivada,"go",label=nombre[0])
    ax[1].set_ylabel("$\dfrac{d CO_2}{dt}$")
    ax[1].set_xlabel("Fecha [dd hh:mm]")
    if zoom:
        ax[1].set_ylim(0,120)
        ax[0].set_ylim(400,1000)
    for figura in range(2):
        ax[figura].grid()
        ax[figura].legend() 
        
def obtiene_resultados(tmp,fluke,sensor,int_promedio):
    TR  = tmp.transcurrido[tmp.derivada<=2]
#     print(TR)
    M   = tmp[sensor].rolling(int_promedio).mean()
    D_r = fluke.fluke.rolling(int_promedio).mean() 
    Error= M.iloc[-1] - D_r.iloc[-1]
    nombre = str(int(D_r.iloc[-1]))+ 'ppm'
    my_dict= {"Error":[abs(Error)],
              "TR+":[TR.iloc[0]],  
              "M":[M[-1]],
              "D_r":[D_r.iloc[-1]],
              'sensor':[sensor],
              'concentracion':[nombre]
             }
    resultados = pd.DataFrame.from_dict(my_dict) 
    resultados.set_index(['sensor','concentracion'],inplace=True)
    return resultados


def importa_fluke(archivo):
    tmp = pd.read_csv(archivo,usecols=[7,11],names=["fluke","tiempo"],skiprows=1)
    tmp.tiempo = pd.to_datetime(tmp.tiempo,dayfirst=True)
    tmp.set_index("tiempo",inplace=True)
    tmp.fluke = tmp.fluke.str.replace(" ppm","")
    tmp.fluke = tmp.fluke.astype("float64")
    return tmp

