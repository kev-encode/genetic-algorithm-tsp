import tsplib95
import shutil
from pathlib import Path


BASE_DIR = Path(__file__).resolve().parent.parent

# note: paths were changed
TSP_DIR = BASE_DIR / "res" / "instances" / "other"
EUC2D_DIR = BASE_DIR / "res" / "instances" / "euc2d"

if __name__ == "__main__":
    for item in TSP_DIR.iterdir():
        if item.is_file():
            instance = tsplib95.load(item)
            if instance.edge_weight_type == "EUC_2D":
                shutil.move(item, EUC2D_DIR)