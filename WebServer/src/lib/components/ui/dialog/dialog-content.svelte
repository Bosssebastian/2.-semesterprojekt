<script lang="ts">
	import { cn, type WithElementRef } from "$lib/utils.js";
	import type { HTMLAttributes } from "svelte/elements";

	type Props = WithElementRef<HTMLAttributes<HTMLDivElement>, HTMLDivElement>;

	let { class: className, ref = $bindable(null), children, ...restProps }: Props = $props();
</script>

<div class="fixed inset-0 bg-slate-950/40 backdrop-blur-[2px]"></div>
<div class="fixed inset-0 flex items-center justify-center px-4">
	<div
		bind:this={ref}
		data-slot="dialog-content"
		role="dialog"
		aria-modal="true"
		class={cn(
			"bg-background text-foreground w-full max-w-md rounded-[2rem] border border-border/80 p-6 shadow-[0_30px_80px_-30px_rgba(15,23,42,0.45)]",
			className
		)}
		{...restProps}
	>
		{@render children?.()}
	</div>
</div>
