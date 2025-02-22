"use client";

import React, { useEffect, useState } from "react";
import { ThemeSpinner } from "./util";

export default function LoadingScreen() {
  const [loss, setLoss] = useState<number | null>(null);

  useEffect(() => {
    const intervalId = setInterval(() => {
      const controller = new AbortController();
      
      fetch('/train/query_loss', {
        signal: controller.signal
      })
        .then(res => res.json())
        .then(data => setLoss(data.loss))
        .catch(err => {
          if (err.name !== 'AbortError') {
            console.error('Failed to fetch loss:', err);
          }
        });

      return () => controller.abort();
    }, 2000);

    return () => clearInterval(intervalId);
  }, []);

  return (
    <div className="flex flex-col items-center justify-center h-screen">
      <ThemeSpinner size="lg" />
      <p className="mt-4 text-xl">
        Model loss: {loss !== null ? loss.toFixed(4) : 'Loading...'}
      </p>
    </div>
  );
}
