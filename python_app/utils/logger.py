import logging, os
if not os.path.exists("logs"): os.makedirs("logs")
logging.basicConfig(filename="logs/app.log",level=logging.INFO,format="%(asctime)s - %(message)s",datefmt="%Y-%m-%d %H:%M:%S")
def log_event(m): logging.info(m); print(m)
