import torch
import torch.nn as nn
import torch.nn.functional as F


class GDN(nn.Module):
    """Generalized Divisive Normalization Layer"""

    def __init__(self, channels, inverse=False):
        super().__init__()
        self.inverse = inverse
        self.beta = nn.Parameter(torch.ones(1, channels, 1, 1))
        self.gamma = nn.Parameter(torch.eye(channels).view(channels, channels, 1, 1))

    def forward(self, x):
        if self.inverse:
            return x * torch.sqrt(self.beta + F.conv2d(x ** 2, self.gamma, padding=0))
        return x / torch.sqrt(self.beta + F.conv2d(x ** 2, self.gamma, padding=0))


class CompressionVAE(nn.Module):
    

    def __init__(self, N=64, M=192):
        super().__init__()
        
        self.encoder = nn.Sequential(
            nn.Conv2d(1, N, 5, stride=2, padding=2),
            GDN(N),
            nn.Conv2d(N, N, 5, stride=2, padding=2),
            GDN(N),
            nn.Conv2d(N, N, 5, stride=2, padding=2),
            GDN(N),
            nn.Conv2d(N, M, 5, stride=2, padding=2)
        )

        
        self.decoder = nn.Sequential(
            nn.ConvTranspose2d(M, N, 5, stride=2, padding=2, output_padding=1),
            GDN(N, inverse=True),
            nn.ConvTranspose2d(N, N, 5, stride=2, padding=2, output_padding=1),
            GDN(N, inverse=True),
            nn.ConvTranspose2d(N, N, 5, stride=2, padding=2, output_padding=1),
            GDN(N, inverse=True),
            nn.ConvTranspose2d(N, 1, 5, stride=2, padding=2, output_padding=1),
            nn.Sigmoid()
        )

    def forward(self, x):
        y = self.encoder(x)
        
        if self.training:
            noise = torch.rand_like(y) - 0.5
            y_quant = y + noise
        else:
            y_quant = torch.round(y)
        x_hat = self.decoder(y_quant)
        return x_hat, y_quant