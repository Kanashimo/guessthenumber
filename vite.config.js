import { defineConfig} from 'vite'
export default defineConfig({
    root: 'client',
    publicDir: 'static',
    server: {
        port: 80,
    },
});