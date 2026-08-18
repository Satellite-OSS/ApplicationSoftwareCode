import yaml
import torch
import torch.nn.functional as F
from skimage import data
from skimage.transform import resize
import numpy as np
from models.vae import CompressionVAE
import argparse
from pathlib import Path
from torch.nn.utils import clip_grad_norm_


def main(config_path):

    config_path = Path(config_path)
    
    with open(config_path) as f:
        config = yaml.safe_load(f)

    
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model = CompressionVAE(**config["model"]).to(device)

    
    image = data.camera()  
    image = resize(image, (256, 256))  
    image_tensor = torch.from_numpy(image).float().unsqueeze(0).unsqueeze(0).to(device)  # shape: [0,0,256,256]

   
    optimizer = torch.optim.Adam(model.parameters(), lr=config["training"]["lr"])

    
    Path("checkpoints").mkdir(exist_ok=True)
    Path("outputs").mkdir(exist_ok=True)

    
    for epoch in range(config["training"]["epochs"]):
        model.train()
        total_loss = 0.0

       
        for _ in range(100):  
            optimizer.zero_grad()

            
            x_hat, y_quant = model(image_tensor)

            
            mse_loss = F.mse_loss(x_hat, image_tensor)
            bpp = torch.log2(torch.abs(y_quant) + 1e-8).mean()
            loss = config["training"]["lambda"] * mse_loss + torch.abs(bpp)

            
            loss.backward()
            clip_grad_norm_(model.parameters(), max_norm=1.0)
            optimizer.step()

            total_loss += loss.item()

        
        avg_loss = total_loss / 100
        print(f"Epoch {epoch + 1}/{config['training']['epochs']} | Loss: {avg_loss:.4f}")

        
        if (epoch + 1) % 10 == 0:
            with torch.no_grad():
                recon_img = x_hat.squeeze().cpu().numpy()
                np.save(f"outputs/recon_{config_path.stem}_epoch{epoch + 1}.npy", recon_img)

            
            torch.save(model.state_dict(), f"checkpoints/{config_path.stem}_epoch{epoch + 1}.pth")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--config", type=str, required=True,
                        help="Path to config file (e.g., configs/low_rate.yaml)")
    args = parser.parse_args()
    main(args.config)