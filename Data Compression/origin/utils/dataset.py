from torch.utils.data import Dataset
from PIL import Image
import numpy as np
import os


class SatelliteDataset(Dataset):
    def __init__(self, img_dir, transform=None):
        self.img_dir = img_dir
        self.img_list = os.listdir(img_dir)
        self.transform = transform

    def __len__(self):
        return len(self.img_list)

    def __getitem__(self, idx):
        img_path = os.path.join(self.img_dir, self.img_list[idx])
        image = Image.open(img_path).convert('L')
        image = np.array(image).astype(np.float32) / 255.0
        if self.transform:
            image = self.transform(image)
        return torch.from_numpy(image).unsqueeze(0)